"""Extract registered commands and adjudicate direct controls and launch options.

Registered ``CommandClass`` metadata comes from the exact objects added to
``AllCommands``. Direct key handlers and command-line branches are deliberately
kept behind a strict adapter: the scanner owns discovery, while the manifest
supplies the short public explanation that source syntax cannot express.
"""

from dataclasses import dataclass
from pathlib import Path
import ast
import re

import yaml

import extract_engine
import ini_inventory
import schema_validation


ROOT = Path(__file__).resolve().parents[2]
CODE = ROOT / "code"
MANIFEST = ROOT / "manual" / "data" / "command-adapters.yaml"
INIT_SOURCE = CODE / "init.cpp"
LANGUAGE_SOURCE = CODE / "language" / "language.rc"

ALL_BUILDS = ["release", "debug"]
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl"}
DIRECT_KEY_CALLS = re.compile(
    r"\b(?:Keyboard\s*->\s*Down|GetKeyState|GetAsyncKeyState|RegisterHotKey|MapVirtualKey)"
    r"\s*\(([^;]*)\)"
)


@dataclass(frozen=True)
class Site:
    file: str
    function: str
    expression: str
    guard: str | None
    line: int
    context: str

    @property
    def identity(self):
        return (self.file, self.function, self.expression, self.guard)

    @property
    def source(self):
        return f"{self.file}:{self.line}"


def read(path):
    return Path(path).read_text(encoding="latin-1")


def matching_brace(text, opening):
    return ini_inventory._matching_brace(text, opening)


def matching_parenthesis(text, opening):
    depth = 0
    quote = None
    escaped = False
    for index in range(opening, len(text)):
        character = text[index]
        if quote is not None:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == quote:
                quote = None
            continue
        if character in {"'", '"'}:
            quote = character
        elif character == "(":
            depth += 1
        elif character == ")":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError("unbalanced condition parentheses")


def function_span(text, name):
    matches = [span for span in ini_inventory.function_spans(text) if span[2] == name]
    if len(matches) != 1:
        raise ValueError(f"expected one definition of {name}, found {len(matches)}")
    return matches[0]


def line_number(text, position):
    return text.count("\n", 0, position) + 1


def route_id(identifier):
    # Launch options publish under /using/command-line/, where the kind
    # prefix would be redundant in the URL.
    if identifier.startswith("launch:"):
        identifier = identifier[len("launch:"):]
    slug = re.sub(r"[^a-z0-9]+", "-", identifier.lower()).strip("-")
    if not slug:
        raise ValueError(f"cannot derive a command route from {identifier!r}")
    return slug


def _decode_rc_string(value):
    try:
        return ast.literal_eval('"' + value + '"')
    except (SyntaxError, ValueError) as error:
        raise ValueError(f"invalid language resource string {value!r}") from error


def language_strings(text=None):
    text = read(LANGUAGE_SOURCE) if text is None else text
    strings = {}
    for match in re.finditer(
            r'^\s*(TXT_[A-Z0-9_]+)\s+"((?:[^"\\]|\\.)*)"\s*$', text, re.M):
        token, value = match.groups()
        strings[token] = _decode_rc_string(value)
    return strings


def class_bodies(text):
    bodies = {}
    for match in re.finditer(
            r"\bclass\s+(\w+CommandClass)\s*:\s*public\s+CommandClass\s*\{", text):
        opening = text.find("{", match.start(), match.end())
        closing = matching_brace(text, opening)
        bodies[match.group(1)] = (text[opening + 1:closing], match.start())
    return bodies


def method_body(class_body, method):
    match = re.search(rf"\b{re.escape(method)}\s*\(\s*void\s*\)\s*const\s*\{{", class_body)
    if not match:
        return None
    opening = class_body.find("{", match.start(), match.end())
    closing = matching_brace(class_body, opening)
    return class_body[opening + 1:closing]


def _method_text(class_name, body, method, resources, team=None, fallback=None):
    source = method_body(body, method)
    if source is None:
        if fallback is not None:
            return fallback
        raise ValueError(f"{class_name}: no supported {method}() definition")
    formatted = re.search(
        r"sprintf\s*\(\s*_cmd_buffer\s*,\s*(?:"
        r'"(?P<literal>(?:[^"\\]|\\.)*)"|'
        r"Fetch_String\s*\(\s*(?P<token>TXT_[A-Z0-9_]+)\s*\))\s*,\s*Team\s*\)",
        source,
    )
    if formatted:
        if team is None:
            raise ValueError(f"{class_name}.{method}: Team format without a team number")
        template = (_decode_rc_string(formatted.group("literal"))
                    if formatted.group("literal") is not None
                    else resources.get(formatted.group("token")))
        if template is None:
            raise ValueError(
                f"{class_name}.{method}: missing language token {formatted.group('token')}")
        try:
            return template % team
        except (TypeError, ValueError) as error:
            raise ValueError(f"{class_name}.{method}: unsupported format {template!r}") from error

    fetched = re.search(
        r"return\s*\(\s*Fetch_String\s*\(\s*\(?\s*(TXT_[A-Z0-9_]+)\s*\)?\s*\)\s*\)",
        source,
    )
    if fetched:
        token = fetched.group(1)
        if token not in resources:
            raise ValueError(f"{class_name}.{method}: missing language token {token}")
        return resources[token]

    literal = re.search(r'return\s*\(\s*"((?:[^"\\]|\\.)*)"\s*\)', source)
    if literal:
        return _decode_rc_string(literal.group(1))
    raise ValueError(f"{class_name}.{method}: unsupported metadata expression")


def _registrations(text, function):
    stripped = extract_engine.strip_comments(text)
    opening, closing, _ = function_span(stripped, function)
    body = stripped[opening + 1:closing]
    aliases = {}
    for match in re.finditer(
            r"(?:CommandClass\s*\*|CommandClass\s+const\s*\*)\s*(\w+)\s*=\s*"
            r"new\s+(\w+CommandClass)\s*(?:\(([^;()]*)\))?\s*;", body):
        aliases[match.group(1)] = (match.group(2), (match.group(3) or "").strip() or None)

    records = []
    for match in re.finditer(r"AllCommands\.Add\s*\(\s*([^;]+?)\s*\)\s*;", body):
        argument = match.group(1).strip()
        direct = re.fullmatch(r"new\s+(\w+CommandClass)\s*(?:\(([^()]*)\))?", argument)
        if direct:
            class_name, parameter = direct.group(1), (direct.group(2) or "").strip() or None
        elif argument in aliases:
            class_name, parameter = aliases[argument]
        else:
            raise ValueError(f"{function}: unsupported AllCommands registration {argument!r}")
        records.append({
            "class": class_name,
            "parameter": parameter,
            "alias": argument if argument in aliases else None,
        })
    return records


def registered_commands(init_text=None, language_text=None):
    raw = read(INIT_SOURCE) if init_text is None else init_text
    stripped = extract_engine.strip_comments(raw)
    resources = language_strings(language_text)
    classes = class_bodies(stripped)
    records = []
    for registration in _registrations(raw, "Init_Commands"):
        class_name = registration["class"]
        if class_name not in classes:
            raise ValueError(f"registered command class {class_name} has no definition")
        body, class_position = classes[class_name]
        parameter = registration["parameter"]
        team = None
        if parameter is not None:
            if not parameter.isdigit():
                raise ValueError(f"{class_name}: unsupported constructor argument {parameter!r}")
            team = int(parameter)
        identifier = _method_text(class_name, body, "Get_Unique_Name", resources, team)
        title = _method_text(
            class_name, body, "Get_Display_Name", resources, team, fallback=identifier)
        category = _method_text(class_name, body, "Get_Category", resources, team)
        description = _method_text(class_name, body, "Get_Description", resources, team)
        records.append({
            "id": identifier,
            "route_id": route_id(identifier),
            "kind": "registered",
            "title": title,
            "description": description,
            "category": category,
            "audience": "player",
            "availability": {
                "builds": list(ALL_BUILDS),
            },
            "_provenance": {
                "source": "code/init.cpp",
                "class": class_name,
                "guard": None,
            },
        })

    by_id = {}
    by_route = {}
    for record in records:
        if record["id"] in by_id:
            raise ValueError(f"duplicate registered command ID {record['id']!r}")
        if record["route_id"] in by_route:
            raise ValueError(
                f"registered command route collision: {record['id']!r} and "
                f"{by_route[record['route_id']]!r}")
        by_id[record["id"]] = record
        by_route[record["route_id"]] = record["id"]

    # These are forced after KEYBOARD.INI is read. No other binding is a
    # source-defined default, so no other record receives a binding field.
    forced = {"DeleteWaypoint": "Delete", "Options": "Escape"}
    for identifier, key in forced.items():
        if identifier not in by_id:
            raise ValueError(f"forced command {identifier!r} is not registered")
        by_id[identifier]["forced_binding"] = key

    if "SpecialWeapons" in by_id:
        raise ValueError("SpecialWeapons is defined but must remain absent until registered")
    return records


def _guard_map(text):
    stack = []
    result = {}
    for number, line in enumerate(text.splitlines(), 1):
        directive = re.match(r"\s*#\s*(ifdef|ifndef|if|else|elif|endif)\b\s*(.*)", line)
        if directive:
            kind, value = directive.groups()
            value = value.strip()
            if kind == "ifdef":
                stack.append(value)
            elif kind == "ifndef":
                stack.append("!" + value)
            elif kind == "if":
                stack.append(re.sub(r"\s+", " ", value))
            elif kind == "else" and stack:
                stack[-1] = f"else({stack[-1]})"
            elif kind == "elif" and stack:
                stack[-1] = re.sub(r"\s+", " ", value)
            elif kind == "endif" and stack:
                stack.pop()
        result[number] = " && ".join(stack) or None
    return result


def _key_expression(value):
    tokens = re.findall(r"\b(?:KN|VK)_[A-Z0-9_]+\b", value)
    return "+".join(dict.fromkeys(tokens)) if tokens else None


def discover_fixed_sites(code_directory=CODE):
    root = Path(code_directory)
    sites = []
    for path in sorted(candidate for candidate in root.rglob("*")
                       if candidate.is_file() and candidate.suffix.lower() in SOURCE_SUFFIXES):
        relative = path.relative_to(root.parent).as_posix()
        raw = read(path)
        stripped = extract_engine.strip_comments(raw)
        guards = _guard_map(raw)
        source_lines = raw.splitlines()
        spans = ini_inventory.function_spans(stripped)

        def add(position, value):
            expression = _key_expression(value)
            if not expression:
                return
            function = ini_inventory.enclosing_function(spans, position)
            if function is None:
                return
            line = line_number(raw, position)
            sites.append(Site(relative, function, expression, guards[line], line,
                              source_lines[line - 1].strip()))

        def add_branches(value, position):
            cursor = 0
            delimiters = [*re.finditer(r"\|\||&&", value), None]
            for delimiter in delimiters:
                end = len(value) if delimiter is None else delimiter.start()
                branch = value[cursor:end]
                key = re.search(r"\b(?:KN|VK)_[A-Z0-9_]+\b", branch)
                branch_position = position + cursor + (key.start() if key else 0)
                add(branch_position, branch)
                cursor = len(value) if delimiter is None else delimiter.end()

        # Switch dispatch is the common input-handler shape. Scan it throughout
        # the source tree rather than assuming which functions own controls.
        for match in re.finditer(r"\bcase\s+(.+?)\s*:", stripped):
            add(match.start(), match.group(1))

        # Walk balanced conditions so truthy bit tests and keys split across
        # several lines cannot evade the inventory.
        for match in re.finditer(r"\b(?:if|while|for|switch)\s*\(", stripped):
            opening = stripped.find("(", match.start(), match.end())
            closing = matching_parenthesis(stripped, opening)
            add_branches(stripped[opening + 1:closing], opening + 1)

        # Comparisons in return statements and assignments are direct tests too.
        for match in re.finditer(r"^.*\b(?:KN|VK)_[A-Z0-9_]+.*$", stripped, re.M):
            line_text = match.group(0)
            if re.search(r"(?:==|!=|<=|>=|(?<!-)[<>])", line_text):
                add_branches(line_text, match.start())

        # State queries and OS registration calls can consume a key without a
        # comparison. Treat each call as a direct-key site as well.
        for match in DIRECT_KEY_CALLS.finditer(stripped):
            add(match.start(), match.group(1))
    unique = {}
    for site in sites:
        unique.setdefault(site.identity, site)
    return sorted(unique.values(), key=lambda site: (site.file, site.line, site.expression))


def discover_launch_sites(init_text=None):
    raw = read(INIT_SOURCE) if init_text is None else init_text
    stripped = extract_engine.strip_comments(raw)
    opening, closing, _ = function_span(stripped, "Parse_Command_Line")
    body = stripped[opening + 1:closing]
    guards = _guard_map(raw)
    source_lines = raw.splitlines()
    sites = []

    def add(expression, position):
        absolute = opening + 1 + position
        line = line_number(raw, absolute)
        sites.append(Site("code/init.cpp", "Parse_Command_Line", expression,
                          guards[line], line, source_lines[line - 1].strip()))

    for match in re.finditer(r"\bcase\s+(PARM_[A-Z0-9_]+)\s*:", body):
        add("obfuscated:" + match.group(1), match.start())
    for match in re.finditer(r"^.*\b(?:stricmp|strcmp|strstr|memcmp|strnicmp)\s*\(.*$", body, re.M):
        line = match.group(0)
        for literal in re.findall(r'"((?:[^"\\]|\\.)*)"', line):
            value = _decode_rc_string(literal)
            if value.startswith(("-", "/")):
                add("literal:" + value, match.start())
            elif value.lower() == ".map":
                add("pattern:*.MAP", match.start())
    for match in re.finditer(r"\bisdigit\s*\(\s*(?:\(\s*unsigned\s+char\s*\)\s*)?string\s*\[\s*1\s*\]\s*\)", body):
        add("pattern:-<width>X<height>", match.start())
    for match in re.finditer(r"\bcase\s+'([A-Z])'\s*:", body):
        add("compact:-X" + match.group(1), match.start())

    unique = {}
    for site in sites:
        unique.setdefault(site.identity, site)
    return sorted(unique.values(), key=lambda site: (site.line, site.expression))


def load_manifest(path=MANIFEST):
    candidate = Path(path)
    with candidate.open(encoding="utf-8") as stream:
        data = yaml.safe_load(stream) or {}
    errors = schema_validation.errors_for(
        data, "command-adapters.schema.json", str(candidate))
    if errors:
        raise ValueError("\n".join(errors))
    return data


def _manifest_identity(site):
    return (site["file"], site["function"], site["expression"], site.get("guard"))


def _exclusion_sites(exclusion):
    return [exclusion["site"]] if "site" in exclusion else exclusion["sites"]


def _adjudicate(discovered, records, exclusions, label):
    by_identity = {site.identity: site for site in discovered}
    claimed = {}
    errors = []
    for record in records:
        for site in record["sites"]:
            identity = _manifest_identity(site)
            claimed.setdefault(identity, []).append(record["id"])
    for exclusion in exclusions:
        for site in _exclusion_sites(exclusion):
            identity = _manifest_identity(site)
            claimed.setdefault(identity, []).append("excluded")

    for identity, owners in claimed.items():
        if len(owners) != 1:
            errors.append(f"{label} adapter {identity}: overlapping classifications {owners}")
        if identity not in by_identity:
            errors.append(f"{label} adapter {identity}: stale classification")
    for identity, site in by_identity.items():
        if identity not in claimed:
            errors.append(
                f"{site.source} {site.function}: {site.expression} has no {label} classification. "
                f"Source: {site.context!r}. Add a public adapter or a reasoned exclusion to "
                "manual/data/command-adapters.yaml")
    if errors:
        raise ValueError("\n".join(errors))
    return by_identity


def adapted_commands(manifest=None, code_directory=CODE):
    manifest = load_manifest() if manifest is None else manifest
    fixed_sites = _adjudicate(
        discover_fixed_sites(code_directory), manifest["fixed_controls"],
        manifest["fixed_exclusions"], "fixed-control")
    launch_sites = _adjudicate(
        discover_launch_sites(), manifest["launch_options"],
        manifest["launch_exclusions"], "launch-option")

    def provenance(record, discovered):
        source = discovered[_manifest_identity(record["sites"][0])]
        return {
            "source": source.file,
            "guard": source.guard,
        }

    fixed = []
    for adapter in manifest["fixed_controls"]:
        fixed.append({
            "id": adapter["id"],
            "route_id": route_id(adapter["id"]),
            "kind": "fixed",
            "title": adapter["title"],
            "description": adapter["description"],
            "audience": adapter["audience"],
            "availability": adapter["availability"],
            "bindings": adapter["bindings"],
            "context": adapter["context"],
            "_provenance": provenance(adapter, fixed_sites),
        })
    launch = []
    for adapter in manifest["launch_options"]:
        record = {
            "id": adapter["id"],
            "route_id": route_id(adapter["id"]),
            "kind": "launch",
            "title": adapter["title"],
            "description": adapter["description"],
            "audience": adapter["audience"],
            "availability": adapter["availability"],
            "syntax": adapter["syntax"],
            "_provenance": provenance(adapter, launch_sites),
        }
        if adapter.get("aliases"):
            record["aliases"] = adapter["aliases"]
        launch.append(record)
    return fixed, launch


def build_catalog(manifest=None):
    registered = registered_commands()
    fixed, launch = adapted_commands(manifest)
    records = [*registered, *fixed, *launch]
    ids = [record["id"] for record in records]
    routes = [record["route_id"] for record in records]
    if len(ids) != len(set(ids)):
        raise ValueError("command catalog contains duplicate IDs")
    if len(routes) != len(set(routes)):
        raise ValueError("command catalog contains duplicate routes")
    result = {
        "registered_commands": registered,
        "fixed_controls": fixed,
        "launch_options": launch,
    }
    errors = schema_validation.errors_for(
        result, "generated-commands.schema.json", "generated command catalog")
    if errors:
        raise ValueError("\n".join(errors))
    return result

