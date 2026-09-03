"""
Layer-1 INI key extractor.

Reads the game source and produces a machine-readable database of every INI key
the engine actually looks up: its name, the file and section it lives in, its
value type, a non-authoritative omission candidate, and which object types it applies to.

The output is deliberately MODDER-FACING. Member names, source line numbers and
class names are recorded for validation and provenance only; the renderer is
expected to drop them from published pages.

Usage:
    python manual/tools/extract.py --report
    python manual/tools/extract.py --out <dir>
"""

import os
import re

import json

import section_selectors


def _yaml():
    """pyyaml, imported on demand.

    Only --show and --out serialize, and pyyaml is not installed everywhere. A
    module-scope import made --report and --show fail outright with
    ModuleNotFoundError, which is how the field-comment pass found this: an agent
    documenting animtype.h could not read the INI facts for its members and had to
    re-derive them from Read_INI by hand.
    """
    import yaml
    return yaml

CODE_DIR = os.path.join(os.path.dirname(__file__), "..", "..", "code")
CODE_DIR = os.path.normpath(CODE_DIR)

# Curated verdicts for keys with more than one meaning; survives regeneration
# because the extractor merges it in rather than storing verdicts in its own
# output. See the file's own header comment for the format.
ADJUDICATIONS_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                  "..", "data", "adjudications.yaml")


# ---------------------------------------------------------------------------
# Modder-facing vocabulary
# ---------------------------------------------------------------------------

# Which INI file an accessor receiver reads from. The generic "ini" receiver
# is resolved from the concrete caller context when one is available.
INI_FILES = {
    "ini": None,            # the file passed in -- resolved per class, see TYPE_FILE
    "ArtINI": "art.ini",
    "ConfigINI": "sun.ini",
    "FSRuleINI": "firestrm.ini",
    "RuleINI": "rules.ini",
    "AI_INI": "ai.ini",
}

# Accessor suffix -> modder-facing value type.
VALUE_TYPES = {
    "Bool": "boolean",
    "Int": "integer",
    "Float": "floating point",
    "Double": "floating point",
    "String": "string",
    # Get_Point and Get_Offset are overloaded on the default's type, so the
    # accessor suffix alone cannot tell a two- from a three-component read.
    # These are the two-component forms; POINT_DIMENSIONS promotes a read whose
    # default resolves to a three-component member.
    "Point": "point (x,y)",
    "Offset": "offset (x,y)",
    "Rect": "rectangle (x,y,w,h)",
    # Get_Lepton parses the authored value in cells (fractions allowed) and
    # stores it multiplied out to leptons, so the modder-facing unit is cells.
    "Lepton": "distance (cells)",
    "VocType": "sound",
    "VocType_List": "list of sounds",
    "VoxType": "EVA speech",
    "VQType": "movie",
    "RGBClass": "colour (R,G,B)",
    "IntList": "list of integers",
    "ArmorType": "ArmorType",
    "CategoryType": "CategoryType",
    "PipScaleType": "PipScaleType",
    "BSizeType": "BSizeType",
    "SuperWeaponType": "SuperWeaponType",
    "TheaterType": "TheaterType",
    "HousesType": "HouseType",
    "RTTIType": "RTTIType",
    "SpeedType": "SpeedType",
    "MPHType": "speed",
    "Side": "Side",
    "CLSID": "Locomotor CLSID",
    "Owners": "list of HouseTypes",
    "Scheme_Index": "colour scheme",
    "BuildingType_List": "list of BuildingTypes",
    "Entry": "string",
    "TextBlock": "text",
    "UUBlock": "binary block",
    "PKey": "public key",
    "Unique_ID": "unique id",
    "Floater_Gravity": "floating point",
    "Speed_Accum": "floating point",
    "Number_Of_Drives": "integer",
    "Color": "colour",
    "Color_Scheme": "colour scheme",
}

# Common C++ section expressions. Reader-specific variables are classified by
# each GLOBAL_UNITS/ADAPTER_UNITS row; nothing unrecognized reaches generated
# output.
SECTION_KINDS = {
    "Name()": section_selectors.identifier("object-type"),
    "IniName": section_selectors.identifier("object-type"),
    "GraphicName": section_selectors.image(),
    "Graphic_Name()": section_selectors.image(),
}

# Concrete type classes -> the modder-facing name for the group they belong to.
CONCRETE_TYPES = {
    "AircraftTypeClass": "AircraftType",
    "BuildingTypeClass": "BuildingType",
    "InfantryTypeClass": "InfantryType",
    "UnitTypeClass": "UnitType",
    "AnimTypeClass": "AnimType",
    "BulletTypeClass": "BulletType",
    "ParticleTypeClass": "ParticleType",
    "ParticleSystemTypeClass": "ParticleSystemType",
    "SmudgeTypeClass": "SmudgeType",
    "TerrainTypeClass": "TerrainType",
    "VoxelAnimTypeClass": "VoxelAnimType",
    "OverlayTypeClass": "OverlayType",
    "IsometricTileTypeClass": "IsometricTileType",
    "WarheadTypeClass": "WarheadType",
    "WeaponTypeClass": "WeaponType",
    "SuperWeaponTypeClass": "SuperWeaponType",
    "HouseTypeClass": "HouseType",
    "TeamTypeClass": "TeamType",
    "TaskForceClass": "TaskForce",
    "ScriptTypeClass": "ScriptType",
    "TriggerTypeClass": "TriggerType",
    "TagTypeClass": "TagType",
    "AITriggerTypeClass": "AITriggerType",
    "TiberiumClass": "Tiberium",
    "CampaignClass": "Campaign",
    "SideClass": "Side",
}

# Which file a given concrete type is read from, when the accessor receiver is
# the generic `ini` parameter.
TYPE_FILE = {
    "AnimTypeClass": "art.ini",
    "IsometricTileTypeClass": "art.ini",
    "TeamTypeClass": "ai.ini or map file",
    "TaskForceClass": "ai.ini or map file",
    "CampaignClass": "battle.ini",
}
DEFAULT_TYPE_FILE = "rules.ini"


# Every direct Read_INI dispatch in RulesClass::Objects.  The collection or
# local pointer identifies the concrete reader; the argument at the call site
# identifies the INI database passed through that reader's inheritance chain.
# Keeping the complete method inventory here makes a new, removed, duplicated,
# or renamed dispatch fail extraction instead of silently changing inherited
# applicability.
RULES_OBJECTS_LOADERS = {
    'HouseTypes': 'HouseTypeClass',
    'SuperWeaponTypes': 'SuperWeaponTypeClass',
    'AnimTypes': 'AnimTypeClass',
    'BuildingTypes': 'BuildingTypeClass',
    'AircraftTypes': 'AircraftTypeClass',
    'UnitTypes': 'UnitTypeClass',
    'InfantryTypes': 'InfantryTypeClass',
    'Weapons': 'WeaponTypeClass',
    'BulletTypes': 'BulletTypeClass',
    '::Warheads': 'WarheadTypeClass',
    'TerrainTypes': 'TerrainTypeClass',
    'SmudgeTypes': 'SmudgeTypeClass',
    'OverlayTypes': 'OverlayTypeClass',
    'ParticleTypes': 'ParticleTypeClass',
    'ParticleSystemTypes': 'ParticleSystemTypeClass',
    'VoxelAnimTypes': 'VoxelAnimTypeClass',
    'miss': 'MissionControlClass',
}

RULES_OBJECTS_READ_RE = re.compile(
    r'(?P<target>(?:::)?[A-Za-z_]\w*)'
    r'(?:\s*\[[^\]]+\])?\s*->\s*Read_INI\s*\(\s*'
    r'(?P<receiver>[A-Za-z_]\w*)\s*\)'
)


# Keys whose names are BUILT at run time. Discovery follows string literals, so
# a formatted name has no literal to find and the settings would be missing
# from the catalog entirely rather than merely misclassified.
#
# A tree-wide sweep found exactly one such reader, so this is a declared
# supplement rather than a general format-string parser: WeaponTypeClass
# ::Read_INI formats "BurstDelay%d" once per element of BurstDelay[] and reads
# each with Get_Int. Both the formatting call and the read are matched below,
# so a change to either fails extraction instead of publishing four keys the
# engine no longer looks up.
COMPUTED_KEY_READS = {
    ("weapon.cpp", "WeaponTypeClass"): [
        {
            "format": r'sprintf\s*\(\s*buf\s*,\s*"BurstDelay%d"\s*,\s*i\s*\)',
            "read": r'BurstDelay\[i\]\s*=\s*ini\.Get_Int\s*\(\s*IniName\s*,'
                    r'\s*buf\s*,\s*BurstDelay\[i\]\s*\)',
            "keys": ["BurstDelay0", "BurstDelay1", "BurstDelay2", "BurstDelay3"],
            "receiver": "ini",
            "suffix": "Int",
            "section_expr": "IniName",
            "member": "BurstDelay",
            "value_type": "integer",
            "default_expr": "BurstDelay[i]",
        },
    ],
}


def computed_key_records(path, cls, raw, body, offset):
    """Expand one declared formatted-name read into ordinary records."""

    records = []
    for supplement in COMPUTED_KEY_READS.get((os.path.basename(path), cls), []):
        for field in ("format", "read"):
            if len(re.findall(supplement[field], body)) != 1:
                raise ValueError(
                    "computed INI key supplement for %s::Read_INI: the %s site "
                    "%r no longer appears exactly once in code/%s; reconcile "
                    "COMPUTED_KEY_READS with the current reader"
                    % (cls, field, supplement[field], os.path.basename(path)))
        match = re.search(supplement["read"], body)
        line = (raw[:offset].count("\n")
                + body[:match.start()].count("\n") + 1)
        for key in supplement["keys"]:
            records.append({
                "key": key,
                "receiver": supplement["receiver"],
                "suffix": supplement["suffix"],
                "section_expr": supplement["section_expr"],
                "default_expr": supplement["default_expr"],
                "local_lhs": False,
                "member": supplement["member"],
                "value_type": supplement["value_type"],
                "declared_in": cls,
                "guard": guard_of(body, match.start()),
                "line": line,
                "src": os.path.basename(path),
            })
    return records


# ---------------------------------------------------------------------------
# Small C++ helpers
# ---------------------------------------------------------------------------

def strip_comments(text):
    """Remove // and /* */ comments without disturbing line numbering."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def split_args(s):
    """Split a C++ argument list on top-level commas."""
    args, depth, cur, i, n = [], 0, [], 0, len(s)
    while i < n:
        c = s[i]
        if c == '"':
            j = i + 1
            while j < n and s[j] != '"':
                j += 2 if s[j] == '\\' else 1
            cur.append(s[i:j + 1])
            i = j + 1
            continue
        if c in "(<[":
            depth += 1
        elif c in ")>]":
            depth -= 1
        if c == "," and depth == 0:
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    if "".join(cur).strip():
        args.append("".join(cur).strip())
    return args


def match_call(text, start):
    """Given index of the '(' after a call name, return (inner, end_index)."""
    depth, i, n = 0, start, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            i += 1
            while i < n and text[i] != '"':
                i += 2 if text[i] == '\\' else 1
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return text[start + 1:i], i
        i += 1
    return None, start


def find_body(text, header_re):
    """Find a brace-balanced function body whose signature matches header_re."""
    m = header_re.search(text)
    if not m:
        return None, None
    brace = text.find("{", m.end())
    if brace < 0:
        return None, None
    depth, i, n = 0, brace, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            i += 1
            while i < n and text[i] != '"':
                i += 2 if text[i] == '\\' else 1
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return text[brace:i + 1], brace
        i += 1
    return None, None


# ---------------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------------

GET_CALL_RE = re.compile(
    r"(?:(?P<recv>[A-Za-z_]\w*)\s*\.\s*)?"
    r"(?P<fn>T?Get_[A-Za-z_]\w*)"
    r"(?:<\s*(?P<targ>[A-Za-z_]\w*)\s*>)?\s*\("
)

# An entry name is whatever stands to the left of '=', and INISection::Find_Entry
# looks it up by the raw bytes of the name, so digits alone are a real name: a
# map file carries its checksum under [Digest] 1. Requiring a leading letter made
# that read invisible to every scanner rather than merely unclassified, so no
# exclusion could be written for it. extract_file, extract_globals.emit and
# ini_inventory.scan_source share this contract and must keep sharing it, or a
# site can be extracted without being inventoried.
KEY_NAME_RE = re.compile(r"\w[\w.]*")

CLASS_RE = re.compile(r"^class\s+([A-Za-z_]\w*)\s*:\s*public\s+([A-Za-z_]\w*)", re.M)

# Member declarations whose type fixes how many components a Get_Point or
# Get_Offset read parses. Coord and Point3D both resolve to TPoint3D<int>;
# Cell and Point2D to a two-component base.
POINT_MEMBER_RE = re.compile(
    r"\b(TPoint3D\s*<[^>]*>|TPoint2D\s*<[^>]*>|Point3D|Point2D|Coord|Cell)\s+"
    r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*;")


def load_point_dimensions():
    """member name -> 3 or 2, scanned from every header."""
    dims = {}
    for name in sorted(os.listdir(CODE_DIR)):
        if not name.endswith((".h", ".hh")):
            continue
        try:
            with open(os.path.join(CODE_DIR, name), encoding="latin-1") as stream:
                text = stream.read()
        except OSError:
            continue
        for typename, member in POINT_MEMBER_RE.findall(text):
            dim = 2 if ("2D" in typename or typename in ("Point2D", "Cell")) else 3
            # A member declared inconsistently across headers is not a safe
            # signal; drop it rather than guess.
            if dims.get(member, dim) != dim:
                dims[member] = None
            else:
                dims.setdefault(member, dim)
    return {k: v for k, v in dims.items() if v}


POINT_DIMENSIONS = load_point_dimensions()


def point_value_type(suffix, default_expr, member):
    """Promote a Point/Offset read to its three-component form when the
    default argument resolves to a three-component member."""
    noun = "point" if suffix == "Point" else "offset"
    for candidate in (default_expr, member):
        if not candidate:
            continue
        # `Weapons[0].FireFLH` identifies its type by the trailing member.
        trailing = re.sub(r"\[[^\]]*\]", "", str(candidate)).split(".")[-1]
        trailing = re.sub(r"^.*->", "", trailing).strip()
        dim = POINT_DIMENSIONS.get(trailing)
        if dim:
            return "%s (x,y,z)" % noun if dim == 3 else "%s (x,y)" % noun
    return None


def load_hierarchy():
    """class -> base, scanned from every header."""
    tree = {}
    for name in os.listdir(CODE_DIR):
        if not name.endswith((".h", ".hh")):
            continue
        try:
            with open(os.path.join(CODE_DIR, name), encoding="latin-1") as stream:
                text = stream.read()
        except OSError:
            continue
        for cls, base in CLASS_RE.findall(text):
            tree[cls] = base
    return tree


def depth_of(tree, cls):
    """How far cls sits below the root of its hierarchy (base classes read first)."""
    depth, seen = 0, set()
    while cls in tree and cls not in seen:
        seen.add(cls)
        cls = tree[cls]
        depth += 1
    return depth


def descendants(tree, root):
    """Every class that derives from root, plus root itself."""
    out = {root}
    changed = True
    while changed:
        changed = False
        for cls, base in tree.items():
            if base in out and cls not in out:
                out.add(cls)
                changed = True
    return out


def ancestry(tree, cls):
    """Every base class above cls."""
    out, seen = set(), set()
    while cls in tree and cls not in seen:
        seen.add(cls)
        cls = tree[cls]
        out.add(cls)
    return out


def related(tree, a, b):
    """
    Do two reading classes see the SAME member under one name?  Only when one
    is the other or its ancestor -- same-named members on unrelated classes
    (AnimTypeClass::Damage vs ParticleTypeClass::Damage) are distinct settings
    that must never merge into one scope.
    """
    return a == b or a in ancestry(tree, b) or b in ancestry(tree, a)


def discover_rules_objects_loaders(code_dir=CODE_DIR, tree=None):
    '''Return the complete, source-ordered RulesClass::Objects dispatch list.

    The inventory is intentionally closed.  An extractor update must classify
    any added or renamed dispatch, while a removed or duplicated dispatch must
    be reconciled here instead of silently changing inherited key scopes.
    '''
    tree = tree or load_hierarchy()
    path = os.path.join(code_dir, 'rules.cpp')
    try:
        with open(path, encoding='latin-1') as stream:
            raw = stream.read()
    except OSError as error:
        raise ValueError(
            'RulesClass::Objects loader inventory: cannot read code/rules.cpp: %s'
            % error) from error

    text = strip_comments(raw)
    header = re.compile(r'\bbool\s+RulesClass::Objects\s*\(')
    body, offset = find_body(text, header)
    if body is None:
        raise ValueError(
            'RulesClass::Objects loader inventory: function body not found')

    broad_sites = list(re.finditer(r'->\s*Read_INI\s*\(', body))
    matches = list(RULES_OBJECTS_READ_RE.finditer(body))
    errors = []
    if len(matches) != len(broad_sites):
        errors.append(
            '%d Read_INI dispatch(es) could not be classified'
            % (len(broad_sites) - len(matches)))

    seen = {}
    rows = []
    for match in matches:
        target = match.group('target')
        receiver = match.group('receiver')
        line = raw[:offset].count('\n') + body[:match.start()].count('\n') + 1
        if target in seen:
            errors.append(
                'duplicate loader %s at code/rules.cpp:%d (first at line %d)'
                % (target, line, seen[target]))
            continue
        seen[target] = line

        reader = RULES_OBJECTS_LOADERS.get(target)
        if reader is None:
            errors.append(
                'unmapped loader %s at code/rules.cpp:%d' % (target, line))
            continue
        if receiver not in INI_FILES:
            errors.append(
                'unclassified INI receiver %s for %s at code/rules.cpp:%d'
                % (receiver, target, line))
            continue
        rows.append({
            'target': target,
            'class': reader,
            'receiver': receiver,
            'source': 'code/rules.cpp:%d' % line,
        })

    for target in RULES_OBJECTS_LOADERS:
        if target not in seen:
            errors.append('expected loader %s is no longer present' % target)

    object_classes = descendants(tree, 'ObjectTypeClass')
    concrete_rows = [row for row in rows if row['class'] in object_classes]
    missing_public = sorted(
        row['class'] for row in concrete_rows
        if row['class'] not in CONCRETE_TYPES)
    if missing_public:
        errors.append(
            'ObjectType loader classes lack public mappings: %s'
            % ', '.join(missing_public))
    duplicate_classes = sorted({
        row['class'] for row in concrete_rows
        if sum(other['class'] == row['class'] for other in concrete_rows) > 1
    })
    if duplicate_classes:
        errors.append(
            'ObjectType classes have more than one loader: %s'
            % ', '.join(duplicate_classes))

    if errors:
        raise ValueError(
            'RulesClass::Objects loader inventory:\n  - '
            + '\n  - '.join(errors))
    return rows


def object_type_loader_contexts(code_dir=CODE_DIR, tree=None):
    '''Return concrete ObjectType readers and their actual INI receivers.'''
    tree = tree or load_hierarchy()
    object_classes = descendants(tree, 'ObjectTypeClass')
    return [
        {
            **row,
            'applies_to': CONCRETE_TYPES[row['class']],
        }
        for row in discover_rules_objects_loaders(code_dir, tree)
        if row['class'] in object_classes
    ]


def parse_ctor_defaults(text, cls):
    """member -> default expression, from the constructor initialiser list."""
    defaults = {}
    for m in re.finditer(re.escape(cls) + r"::" + re.escape(cls) + r"\s*\(", text):
        inner, end = match_call(text, m.end() - 1)
        if inner is None:
            continue
        params = {p.split("=")[0].split()[-1].lstrip("*&")
                  for p in split_args(inner) if p.split("=")[0].split()}
        brace = text.find("{", end)
        colon = text.find(":", end)
        if colon < 0 or (brace >= 0 and colon > brace):
            continue
        init = text[colon + 1:brace]
        for item in split_args(init):
            im = re.match(r"^([A-Za-z_]\w*)\s*\((.*)\)$", item, re.S)
            if im and im.group(1) not in ("BASECLASS",):
                expr = " ".join(im.group(2).split())
                # A bare constructor parameter is a passthrough, not a default.
                if expr in params:
                    continue
                defaults.setdefault(im.group(1), expr)
    return defaults



def guard_of(text, pos):
    """Nearest enclosing #ifdef at a source position, if any."""
    guard, depth = None, 0
    for m in re.finditer(r"^\s*#\s*(ifdef|ifndef|if|endif)\b[ \t]*(\S*)", text[:pos], re.M):
        kind, arg = m.group(1), m.group(2)
        if kind == "endif":
            depth = max(0, depth - 1)
            if depth == 0:
                guard = None
        else:
            depth += 1
            if depth == 1 and kind == "ifdef":
                guard = arg
    return guard


def extract_file(path, cls, tree):
    """Pull every key read by cls::Read_INI out of one translation unit."""
    with open(path, encoding="latin-1") as stream:
        raw = stream.read()
    text = strip_comments(raw)

    header = re.compile(r"\bbool\s+" + re.escape(cls) + r"::Read_INI\s*\(")
    body, offset = find_body(text, header)
    if body is None:
        return [], {}

    defaults = parse_ctor_defaults(text, cls)
    records = computed_key_records(path, cls, raw, body, offset)

    for m in GET_CALL_RE.finditer(body):
        inner, end = match_call(body, m.end() - 1)
        if inner is None:
            continue
        args = split_args(inner)
        if not args:
            continue

        recv, fn, targ = m.group("recv"), m.group("fn"), m.group("targ")
        suffix = fn[5:] if fn.startswith("TGet_") else fn[4:]

        # Free-function forms take the INI object as their first argument.
        if recv is None:
            if not args or not re.match(r"^[A-Za-z_]\w*$", args[0]):
                continue
            recv, args = args[0], args[1:]
        # Get_BuildingType_List(ini, section, ...) passes the file twice.
        if args and args[0] in INI_FILES:
            args = args[1:]

        # Every accessor spells the section first and the entry second, so a
        # literal further along the list is a default, a scratch buffer, or a
        # size -- never an entry name.
        if len(args) < 2 or not args[1].startswith('"'):
            continue
        key = args[1][1:-1]
        if not KEY_NAME_RE.fullmatch(key):
            continue

        section = args[0]
        default = args[2] if len(args) > 2 else None
        # Get_String's "default" slot is a scratch buffer; the real default is
        # the destination member, which the caller pre-loads.
        if suffix == "String" and len(args) > 3:
            default = args[3]

        lhs = None
        local_lhs = False
        pre = body[max(0, m.start() - 120):m.start()]
        # An intervening C cast (`X = (AddonType)ini.Get_Int(...)`) must not
        # hide the assignment target.
        lm = re.search(
            r"([A-Za-z_][\w\[\]\.\->]*)\s*=\s*(?:\(\s*[A-Za-z_]\w*\s*\*?\s*\)\s*)?$",
            pre)
        if lm:
            lhs = lm.group(1)
            # A type token directly before the target means a fresh LOCAL
            # (`int delay = ini.Get_Int(..., -1)`): the sentinel-staging
            # idiom, whose default argument is NOT the key's default.
            local_lhs = re.search(r"\w[\w:<>]*[ \t]+$", pre[:lm.start(1)]) is not None

        member = lhs or (default if default and re.match(r"^[A-Za-z_]\w*$", default or "") else None)
        member = re.sub(r"\[\d+\].*$", "", member) if member else None

        value_type = VALUE_TYPES.get(suffix)
        if suffix in ("Point", "Offset"):
            value_type = point_value_type(suffix, default, member) or value_type
        if value_type is None and targ:
            value_type = "list of %ss" % CONCRETE_TYPES.get(targ, targ)
        if value_type is None:
            value_type = suffix.replace("_", " ").lower()
        if fn.startswith("TGet_") and targ and "list" not in value_type.lower():
            value_type = "%s" % CONCRETE_TYPES.get(targ, targ)

        records.append({
            "key": key,
            "receiver": recv,
            "suffix": suffix,
            "section_expr": section,
            "default_expr": default,
            "local_lhs": local_lhs,
            "member": member,
            "value_type": value_type,
            "declared_in": cls,
            "guard": guard_of(body, m.start()),
            "line": raw[:offset].count("\n") + body[:m.start()].count("\n") + 1,
            "src": os.path.basename(path),
        })

    return records, defaults


# ---------------------------------------------------------------------------
# Assembly
# ---------------------------------------------------------------------------

SECTION_CONST_RE = re.compile(
    r"(?:static\s+)?char\s+const\s*\*\s*(?:const\s+)?([A-Z_][A-Z0-9_]*)\s*=\s*\"([^\"]+)\"")


def extract_globals(path, cls, methods, opts=None):
    """
    Pull keys out of non-hierarchy readers: the global rules sections
    ([General], [AudioVisual], ...), scenario/map readers, sun.ini readers,
    per-entry sound/theme readers.

    Typed Get_* accessor calls are the source of truth for public settings.

    opts:
        file         -- INI file label when the receiver doesn't decide it
        section_vars -- {C++ expression: public selector object}
        group        -- applies_to label for these records

    A method spelling may carry the opening of its parameter list --
    ``":MultiMission(INIClass"`` -- to pick one overload out of several
    definitions that share a name. Overloads are separate readers with separate
    public scopes, so each is registered as its own unit rather than merged.
    """
    opts = opts or {}
    with open(path, encoding="latin-1") as stream:
        raw = stream.read()
    text = strip_comments(raw)
    sections = dict(SECTION_CONST_RE.findall(text))
    defaults = parse_ctor_defaults(text, cls)
    records = []

    for method in methods:
        if "::" in method:
            owner, name = method.split("::")
        elif method[0] == ":":            # ":Free_Function"
            owner, name = None, method[1:]
        else:
            owner, name = cls, method
        name, _, signature = name.partition("(")
        pat = (re.escape(owner) + r"::" if owner else "") + re.escape(name)
        header = re.compile(
            r"^[A-Za-z_][^\n;{}]*?\b" + pat + r"\s*\(\s*"
            + r"\s+".join(re.escape(token) for token in signature.split()),
            re.M)
        body, offset = find_body(text, header)
        if body is None:
            continue

        def emit(recv, suffix, section_expr, key, member, pos,
                 default_expr=None, local_lhs=False):
            if not KEY_NAME_RE.fullmatch(key or ""):
                return
            line = raw[:offset].count("\n") + body[:pos].count("\n") + 1
            section = section_selectors.classify(
                section_expr,
                constants=sections,
                variables=opts.get("section_vars"),
                default_identifier=opts.get(
                    "default_section_source", "object-type"),
                context="code/%s:%d key %r" % (
                    os.path.basename(path), line, key),
            )
            records.append({
                "key": key,
                "receiver": recv,
                "suffix": suffix,
                "section_expr": section,
                "default_expr": default_expr if default_expr is not None else member,
                "local_lhs": local_lhs,
                "member": member,
                "value_type": VALUE_TYPES.get(suffix, suffix.replace("_", " ").lower()),
                "declared_in": cls,
                "guard": guard_of(body, pos),
                "line": line,
                "src": os.path.basename(path),
                "group": opts.get("group", "global rules"),
                "file_hint": opts.get("file"),
            })

        # Typed accessor reads in the selected bodies.
        for m in GET_CALL_RE.finditer(body):
            inner, _ = match_call(body, m.end() - 1)
            if inner is None:
                continue
            args = split_args(inner)
            recv, fn = m.group("recv"), m.group("fn")
            if recv is None:
                if not args or args[0] not in INI_FILES:
                    continue
                recv, args = args[0], args[1:]
            # Get_VocType_List(ini, section, ...) passes the file twice.
            if args and args[0] in INI_FILES:
                args = args[1:]
            suffix = fn[5:] if fn.startswith("TGet_") else fn[4:]
            # The entry is always the argument after the section, whether the
            # section is a literal -- Get_X("Options", "GameSpeed", ...) -- or a
            # variable. A literal past that position is a default value.
            if len(args) < 2 or not args[1].startswith('"'):
                continue
            section_expr, key = args[0], args[1][1:-1]
            default_expr = args[2] if len(args) > 2 else None
            pre = body[max(0, m.start() - 120):m.start()]
            lm = re.search(
                r"([A-Za-z_]\w*)(?:\.\w+)?\s*=\s*(?:\(\s*[A-Za-z_]\w*\s*\*?\s*\)\s*)?$",
                pre)
            local_lhs = bool(
                lm and re.search(r"\w[\w:<>]*[ \t]+$", pre[:lm.start(1)]))
            emit(recv, suffix, section_expr, key,
                 lm.group(1) if lm else None, m.start(),
                 default_expr=default_expr, local_lhs=local_lhs)

    return records, defaults


GLOBAL_UNITS = [
    ("rules.cpp", "RulesClass", [
        "General", "Audio_Visual_Rules", "AI", "Combat_Damage", "IQ",
        "MPlayer", "Crate_Rules", "Jumpjet_Controls", "Heap_Maximums",
        "Difficulty_Rules", "Land_Characteristics",
    ], None),
    ("scenario.cpp", "ScenarioClass", [
        "Read_INI", "Read_Global_INI", "Read_Local_INI", "Read_Waypoints",
        ":Read_Scenario_INI",
    ], {"file": "map file", "group": "scenarios"}),
    ("special.cpp", "SpecialClass", ["Read_INI"],
     {"file": "map file", "group": "scenarios"}),
    ("house.cpp", "HouseClass", ["Read_INI"],
     {"file": "map file", "group": "House (per-scenario)",
      "section_vars": {"hname": section_selectors.identifier("house")}}),
    ("session.cpp", "SessionClass", ["Read_MultiPlayer_Settings"],
     {"file": "sun.ini", "group": "multiplayer settings"}),
    # Only the loose *.MPR directory scan reads settings here. The packet path
    # above it builds each entry through MultiMission's own constructor, which
    # reads a different section from a different file and is enrolled as its
    # own adapter unit.
    ("session.cpp", "SessionClass", ["Read_Scenario_Descriptions"],
     {"file": "map file (.mpr)", "group": "multiplayer maps"}),
    ("options.cpp", "OptionsClass", ["Load_Settings"],
     {"group": "client settings"}),
    ("voc.cpp", "VocClass", ["Fill_In"],
     {"file": "sound01.ini", "group": "Sounds",
      "section_vars": {"Name": section_selectors.identifier("sound"),
                       "Name()": section_selectors.identifier("sound"),
                       "IniName": section_selectors.identifier("sound")}}),
    ("theme.cpp", "ThemeControl", ["Fill_In"],
     {"file": "theme01.ini", "group": "Themes",
      "section_vars": {"Name": section_selectors.identifier("theme"),
                       "Name()": section_selectors.identifier("theme"),
                       "IniName": section_selectors.identifier("theme")}}),
]


UNITS = [
    ("abstype.cpp", "AbstractTypeClass"),
    ("objtype.cpp", "ObjectTypeClass"),
    ("techtype.cpp", "TechnoTypeClass"),
    ("builtype.cpp", "BuildingTypeClass"),
    ("airctype.cpp", "AircraftTypeClass"),
    ("infatype.cpp", "InfantryTypeClass"),
    ("unittype.cpp", "UnitTypeClass"),
    ("animtype.cpp", "AnimTypeClass"),
    ("bullettype.cpp", "BulletTypeClass"),
    ("overtype.cpp", "OverlayTypeClass"),
    ("smudtype.cpp", "SmudgeTypeClass"),
    ("terrtype.cpp", "TerrainTypeClass"),
    ("ptype.cpp", "ParticleTypeClass"),
    ("psystype.cpp", "ParticleSystemTypeClass"),
    ("houstype.cpp", "HouseTypeClass"),
    ("suprtype.cpp", "SuperWeaponTypeClass"),
    ("isotype.cpp", "IsometricTileTypeClass"),
    ("vanimtype.cpp", "VoxelAnimTypeClass"),
    ("weapon.cpp", "WeaponTypeClass"),
    ("warhead.cpp", "WarheadTypeClass"),
    ("tiberium.cpp", "TiberiumClass"),
    ("teamtype.cpp", "TeamTypeClass"),
    ("taskforc.cpp", "TaskForceClass"),
    ("campaign.cpp", "CampaignClass"),
]


def resolve_default(rec, all_defaults, tree):
    """Find a constructor candidate on the declaring class or its base chain.

    Several unrelated readers use the same member spelling. Searching every
    constructor map can therefore manufacture a plausible but false value. At
    each ancestry level, accept the expression only when every matching reader
    agrees; otherwise leave the editorial candidate unknown.
    """
    member = rec["member"]
    cls = rec["declared_in"]
    seen = set()
    while cls and cls not in seen:
        seen.add(cls)
        matches = {
            defaults[member]
            for reader, defaults in all_defaults.items()
            if reader.split("/", 1)[0] == cls and member in defaults
        } if member else set()
        if len(matches) == 1:
            return matches.pop()
        if len(matches) > 1:
            return None
        cls = tree.get(cls)
    # No constructor default -- a LITERAL default argument in the read itself
    # is authoritative (`Get_Int(hname, "Credits", 0)`) UNLESS the read stages
    # into a fresh local, where the literal is usually a sentinel the code
    # tests before applying (`int delay = Get_Int(..., -1)`). A mixed-case
    # identifier is the current-value idiom and an expression is opaque;
    # neither is a reliable omission candidate.
    if rec.get("local_lhs"):
        return None
    expr = rec.get("default_expr")
    if expr and re.match(
            r'^(-?[\d.]+f?|"[^"]*"|[A-Z][A-Z0-9_]*|true|false)$', expr):
        return expr
    return None


# Code constants translated to a possible modder-facing omission candidate --
# each spelling verified against the parser that reads the key
# (Get_SourceType/Get_BSizeType/... parse these names; plain-integer keys get
# the enum's numeric value). An unmapped constant yields no candidate rather
# than leaking code-speak into editorial review evidence.
KNOWN_DEFAULTS = {
    "NULL": "none",
    "MPH_IMMOBILE": "0",
    "BSIZE_11": "1x1",
    "BSIZE_FIRST": "1x1",
    "SOURCE_NORTH": "North",
    "CRATE_MONEY": "Money",
    "CRATE_HEAL_BASE": "HealBase",
    "LAND_CLEAR": "Clear",
    "MZONE_NORMAL": "Normal",
    "PIP_GREEN": "green",
    "DIFF_NORMAL": "1",
    "DIR_N": "0",
    "CALL_WAIT_CUSTOM": "3",
    "MAX_PLAYERS": "8",
    "CLSID_TeleportLocomotion": "the Teleport locomotor",
    "CELL_LEPTON_W": "256",
    "4*CELL_LEPTON_W": "1024",
    "9*CELL_LEPTON_W": "2304",
    "CELL_LEPTON": "256",
    "CELL_LEPTON/2": "128",
    "3*CELL_LEPTON/2": "384",
    "2*CELL_LEPTON": "512",
    "5*CELL_LEPTON/2": "640",
    "6*CELL_LEPTON": "1536",
    "32*CELL_LEPTON": "8192",
    "FACING_COUNT": "8",
    "2*TICKS_PER_SECOND": "30",
    "4*TICKS_PER_SECOND": "60",
    "20*TICKS_PER_SECOND": "300",
    "M_PI/2": "1.5708",
    "1.0/60.0": "0.0167",
}


def prettify_default(expr, value_type):
    if expr is None:
        return None
    e = " ".join(expr.split())
    # A quoted string literal is already exactly what the modder would write.
    if len(e) >= 2 and e.startswith('"') and e.endswith('"'):
        return e[1:-1]
    if value_type == "boolean":
        if e in ("false", "0"):
            return "no"
        if e in ("true", "1"):
            return "yes"
    if e == "false":
        return "0"
    if e == "true":
        return "1"
    if e.startswith("Point2D(") or e.startswith("Coord("):
        e = e[e.index("(") + 1:-1]
    e = e.replace(" ", "")
    if e in KNOWN_DEFAULTS:
        return KNOWN_DEFAULTS[e]
    if re.match(r"^[A-Z][A-Z0-9_]*_NONE$", e):
        return "none"
    if re.match(r"^-?0[xX][0-9A-Fa-f]+$", e):
        return str(int(e, 16))
    if re.match(r"^-?[\d.]+f?$", e):
        return e.rstrip("f")
    if re.match(r"^(-?[\d.]+f?,)+-?[\d.]+f?$", e):
        return e.rstrip("f").replace("f,", ",")
    if re.match(r"^(0[xX][0-9A-Fa-f]+,)+0[xX][0-9A-Fa-f]+$", e):
        return ",".join(str(int(p, 16)) for p in e.split(","))
    # An unrecognised identifier or expression is code, not an omission value a
    # modder could type -- retain no candidate rather than something confusing.
    if re.match(r"^[A-Za-z_0-9*/+.()-]+$", e):
        return None
    return expr.strip()


# Readers whose art reads have nothing to fall back to when Image= is absent.
# ObjectTypeClass::Read_INI passes the current GraphicName as its default and
# keeps it, so an art read under GraphicName lands on the object's own entry.
# BulletTypeClass::Read_INI reads Image= a second time with an empty default,
# which wipes that name before its own art reads run.
IMAGE_WITHOUT_FALLBACK_READERS = frozenset({'BulletTypeClass'})


def _section_variables(declared_in):
    if declared_in not in IMAGE_WITHOUT_FALLBACK_READERS:
        return None
    return {
        'GraphicName': section_selectors.image(None),
        'Graphic_Name()': section_selectors.image(None),
    }


def _scope_identity(scope):
    provenance = scope['_provenance']
    return '%s::%s' % (provenance['declared_in'], provenance['member'])


def _route_slug(text):
    return re.sub(r'^-|-+$', '', re.sub(r'[^a-z0-9]+', '-', str(text).lower()))


def _scope_order_signature(scope):
    """Stable ordering for scopes that would share one public route id.

    Route ids come from applies_to, so two scopes of one key with the same
    applicability are told apart only by their position in this list. Position
    is an extraction artifact -- unit order, then discovery order, then source
    order -- so a reader added anywhere earlier would silently swap two
    published routes. Order them by their own recorded content instead. The
    member name is the last resort because a read that names its destination is
    the one the catalog already identifies (adjudications key on
    declared_in::member); an anonymous read sorts behind it.
    """

    provenance = scope['_provenance']
    member = provenance['member']
    return (
        provenance['declared_in'] or '',
        scope['file'],
        scope['value_type'],
        json.dumps(scope['section'], sort_keys=True, separators=(',', ':')),
        json.dumps(sorted(scope['applies_to']), separators=(',', ':')),
        scope.get('note') or '',
        scope.get('precedence') or '',
        member is None,
        member or '',
    )


def _order_shared_route_scopes(key, scopes):
    """Sort only the scopes that collide on one route id, in place of position."""

    groups = {}
    for index, scope in enumerate(scopes):
        base = _route_slug(scope['applies_to'][0]) if scope['applies_to'] else ''
        groups.setdefault(base, []).append(index)

    ordered = list(scopes)
    for base, indices in groups.items():
        if len(indices) < 2:
            continue
        ranked = sorted(
            (scopes[index] for index in indices), key=_scope_order_signature)
        signatures = [_scope_order_signature(scope) for scope in ranked]
        if len(set(signatures)) != len(signatures):
            raise ValueError(
                '%s: two scopes share the public route id %r and every recorded '
                'field, so their published routes would depend on extraction '
                'order; distinguish the reads or adjudicate one away'
                % (key, base))
        for slot, scope in zip(indices, ranked):
            ordered[slot] = scope
    return ordered


def _same_reader_family(left, right, tree):
    left_provenance = left['_provenance']
    right_provenance = right['_provenance']
    return (
        left_provenance['member'] == right_provenance['member']
        and related(
            tree,
            left_provenance['declared_in'],
            right_provenance['declared_in'],
        )
    )


def _record_contexts(rec, tree, loader_contexts, extraction_unit):
    if rec.get('group'):
        return [{
            'class': None,
            'applies_to': [rec['group']],
            'loader_receiver': None,
            'file_class': extraction_unit,
            'applicability_family': None,
        }]

    declared_in = rec['declared_in']
    object_reader = (
        declared_in == 'ObjectTypeClass'
        or 'ObjectTypeClass' in ancestry(tree, declared_in)
    )
    if object_reader:
        contexts = [
            {
                'class': context['class'],
                'applies_to': [context['applies_to']],
                'loader_receiver': context['receiver'],
                'file_class': context['class'],
                'applicability_family': 'object',
            }
            for context in loader_contexts
            if (
                declared_in == context['class']
                or declared_in in ancestry(tree, context['class'])
            )
        ]
        # Keep rules-backed contexts together and ahead of the AnimType art
        # context so scope and fragment ordering remain deterministic.
        return sorted(
            contexts,
            key=lambda context: (
                context['loader_receiver'] != 'ini',
                context['applies_to'],
            ),
        )

    # Preserve the historical single descendant scope for every unrelated
    # reader.  Only ObjectTypeClass and its derived readers are caller-expanded.
    return [{
        'class': None,
        'applies_to': sorted({
            CONCRETE_TYPES[concrete]
            for concrete in descendants(tree, declared_in)
            if concrete in CONCRETE_TYPES
        }),
        'loader_receiver': None,
        'file_class': extraction_unit,
        'applicability_family': ('legacy', extraction_unit),
    }]


def _scope_file(rec, context):
    receiver = rec['receiver']
    if receiver == 'ini' and context.get('loader_receiver'):
        receiver = context['loader_receiver']
    ini_file = INI_FILES.get(receiver)
    if ini_file is None:
        concrete = context.get('file_class') or rec['declared_in']
        ini_file = (
            rec.get('file_hint')
            or TYPE_FILE.get(concrete, DEFAULT_TYPE_FILE)
        )
    return ini_file


def _fold_supersedes(scope, previous):
    """Which of two identical reads owns the scope they fold into.

    Two readers can make the same read for the same concrete type -- a derived
    Read_INI re-reading a setting its base already read, or one class reading
    one scenario entry from two of its own methods. Keeping whichever arrived
    first made the surviving declaration, and whether the type stayed inside its
    family's scope at all, depend on the order the units happened to be visited
    in, so a unit added to either list could split or merge a published scope.

    Settle it on recorded content: cite where the setting is introduced, which
    is the shallowest declaring class -- the one whose scope covers the widest
    family -- and then that class's earliest read. A repeat of the same read
    into the same member is a re-application, not a second setting, so the later
    site is the weaker citation: PrimaryFireFLH is applied to the elite weapon
    slot after the primary one, and Image is re-read by BulletTypeClass after
    ObjectTypeClass has already read it for every type.
    """

    return scope['_order'] < previous['_order']


def _chain_signature(chain):
    return tuple(
        (
            _scope_identity(scope),
            scope['_provenance']['source'],
            # Provenance cites a file, so the read position distinguishes two
            # reads of one key in one file.
            scope['_order'],
            scope['file'],
            json.dumps(scope['section'], sort_keys=True, separators=(',', ':')),
            scope['value_type'],
        )
        for scope in chain
    )


def build(records_by_cls, defaults_by_cls, tree, loader_contexts=None):
    '''Build one generated entry per key from concrete reader call chains.'''
    if loader_contexts is None:
        loader_contexts = object_type_loader_contexts(tree=tree)

    keys = {}
    applicability_lists = {}

    def shared_applicability(values, family):
        values = sorted(values)
        if family is None:
            return list(values)
        identity = (family, tuple(values))
        return applicability_lists.setdefault(identity, list(values))

    for extraction_unit, records in records_by_cls.items():
        for rec in records:
            default_section_source = (
                'campaign'
                if rec.get('declared_in') == 'CampaignClass'
                else 'object-type'
            )
            section = section_selectors.classify(
                rec['section_expr'],
                variables=_section_variables(rec.get('declared_in')),
                default_identifier=default_section_source,
                context='code/%s:%d key %r' % (
                    rec['src'], rec['line'], rec['key']),
            )
            default_candidate = prettify_default(
                resolve_default(rec, defaults_by_cls, tree), rec['value_type'])

            entry = keys.setdefault(rec['key'], {
                'key': rec['key'],
                'scopes': [],
            })
            for context in _record_contexts(
                    rec, tree, loader_contexts, extraction_unit):
                ini_file = _scope_file(rec, context)
                if section['kind'] == 'image' and ini_file == DEFAULT_TYPE_FILE:
                    ini_file = 'art.ini'
                scope = {
                    'applies_to': shared_applicability(
                        context['applies_to'],
                        context['applicability_family'],
                    ),
                    'file': ini_file,
                    # Concrete caller clones are separate public scopes. Keep
                    # their selector objects independent; read-chain entries
                    # below still share the winning selector as before.
                    'section': dict(section),
                    'value_type': rec['value_type'],
                    'status': 'generated',
                    '_provenance': {
                        'default_candidate': default_candidate,
                        'declared_in': rec['declared_in'],
                        'member': rec['member'],
                        'source': 'code/%s' % rec['src'],
                        'guard': rec['guard'],
                    },
                    '_context': tuple(context['applies_to']),
                    '_applicability_family': context['applicability_family'],
                    '_order': (
                        depth_of(tree, rec['declared_in']),
                        rec['line'],
                    ),
                }

                # A discovered source site is expanded once per concrete caller.
                # Repeated enrollment of that same site must not duplicate a read.
                duplicate = next((
                    previous for previous in entry['scopes']
                    if (
                        previous['_context'] == scope['_context']
                        and _same_reader_family(previous, scope, tree)
                        and previous['file'] == scope['file']
                        and previous['section'] == scope['section']
                        and previous['value_type'] == scope['value_type']
                    )
                ), None)
                if duplicate is None:
                    entry['scopes'].append(scope)
                elif _fold_supersedes(scope, duplicate):
                    duplicate.clear()
                    duplicate.update(scope)

    for entry in keys.values():
        context_chains = []
        for scope in entry['scopes']:
            for chain in context_chains:
                if (
                    chain[0]['_context'] == scope['_context']
                    and _same_reader_family(chain[0], scope, tree)
                ):
                    chain.append(scope)
                    break
            else:
                context_chains.append([scope])

        resolved = []
        for chain in context_chains:
            chain.sort(key=lambda scope: scope['_order'])
            winner = dict(chain[-1])
            winner['_chain'] = list(chain)
            winner['_chain_signature'] = _chain_signature(chain)
            if len(chain) > 1:
                winner['read_from'] = [
                    {
                        'file': scope['file'],
                        'section': scope['section'],
                        'value_type': scope['value_type'],
                        'applies_to': list(scope['applies_to']),
                    }
                    for scope in chain
                ]
                if len({
                    (scope['file'], scope['value_type']) for scope in chain
                }) > 1:
                    winner['precedence'] = (
                        'read more than once -- the %s value is applied last and '
                        'overrides the others' % winner['file']
                    )
                else:
                    winner['precedence'] = (
                        'accepted under more than one entry heading; the last one '
                        'the engine reads wins'
                    )
            resolved.append(winner)

        coalesced = []
        for scope in resolved:
            previous = next((
                candidate for candidate in coalesced
                if candidate['_chain_signature'] == scope['_chain_signature']
            ), None)
            if previous is None:
                coalesced.append(scope)
                continue
            previous['applies_to'] = sorted(set(
                previous['applies_to'] + scope['applies_to']))

        for scope in coalesced:
            family = scope.get('_applicability_family')
            if family == 'object':
                family = 'object-result'
            scope['applies_to'] = shared_applicability(
                scope['applies_to'], family)
            if scope.get('read_from'):
                for read in scope['read_from']:
                    read['applies_to'] = scope['applies_to']

        families = []
        for scope in coalesced:
            for family in families:
                if _same_reader_family(family[0], scope, tree):
                    family.append(scope)
                    break
            else:
                families.append([scope])
        if len(families) > 1:
            for scope in coalesced:
                scope['status'] = 'needs_review'

        for scope in coalesced:
            scope.pop('_context', None)
            scope.pop('_applicability_family', None)
            scope.pop('_order', None)
            scope.pop('_chain', None)
            scope.pop('_chain_signature', None)
        entry['scopes'] = _order_shared_route_scopes(entry['key'], coalesced)

    # Entry lookup compares the raw bytes of the name (INISection::Find_Entry, code/ini.cpp).
    # INI keys are case-sensitive, so near-miss spellings remain distinct and
    # cross-reference one another as a modding warning.
    by_lower = {}
    for name in keys:
        by_lower.setdefault(name.lower(), []).append(name)
    for names in by_lower.values():
        if len(names) > 1:
            for name in names:
                keys[name]['case_collides_with'] = sorted(
                    other for other in names if other != name)
    return keys


def load_adjudications():
    if not os.path.exists(ADJUDICATIONS_PATH):
        return {}
    with open(ADJUDICATIONS_PATH, encoding="utf-8") as fh:
        return _yaml().safe_load(fh) or {}


def apply_adjudications(keys, adjudications):
    '''Merge curated collision verdicts into every matching caller context.

    Stable reader/member identities may now produce more than one scope when
    concrete callers use different files or selectors.  One verdict covers all
    such clones.  A `specializes` relation is the only operation allowed to
    subtract a derived scope from broader applicability, and only after proving
    that both scopes use the same file and section selector.
    '''
    stale = []
    for key, verdicts in adjudications.items():
        entry = keys.get(key)
        if entry is None:
            stale.append('%s: key no longer extracted' % key)
            continue
        verdicts = verdicts or {}

        def scopes_for(identity):
            return [
                scope for scope in entry['scopes']
                if _scope_identity(scope) == identity
            ]

        # Absorption removes every context clone of the duplicate identity.
        for ident, verdict in verdicts.items():
            verdict = verdict or {}
            if not scopes_for(ident):
                stale.append('%s: scope %s not found' % (key, ident))
                continue
            for victim_id in verdict.get('absorbs', []):
                victims = scopes_for(victim_id)
                if not victims:
                    stale.append(
                        '%s: absorbed scope %s not found' % (key, victim_id))
                    continue
                entry['scopes'] = [
                    scope for scope in entry['scopes']
                    if _scope_identity(scope) != victim_id
                ]

        # A derived interpretation specializes a broad interpretation only in
        # the identical public lookup context.  Different files or selectors
        # remain additive even when applicability is a strict subset.
        for ident, verdict in verdicts.items():
            verdict = verdict or {}
            target_id = verdict.get('specializes')
            if not target_id:
                continue
            specialists = scopes_for(ident)
            targets = scopes_for(target_id)
            if not specialists:
                # The missing identity was already reported above.
                continue
            if not targets:
                stale.append(
                    '%s: specialization target %s not found for %s'
                    % (key, target_id, ident))
                continue

            for specialist in specialists:
                narrow = set(specialist['applies_to'])
                candidates = [
                    target for target in targets
                    if (
                        target['file'] == specialist['file']
                        and target['section'] == specialist['section']
                        and narrow < set(target['applies_to'])
                    )
                ]
                if len(candidates) != 1:
                    stale.append(
                        '%s: %s specializes %s only when exactly one broader '
                        'same-file/same-section scope matches (found %d)'
                        % (key, ident, target_id, len(candidates)))
                    continue
                target = candidates[0]
                target['applies_to'] = sorted(
                    set(target['applies_to']) - narrow)
                target['shares_name_with'] = (
                    specialist['_provenance']['member'])

        for ident, verdict in verdicts.items():
            verdict = verdict or {}
            matches = scopes_for(ident)
            if not matches:
                # Missing identities were reported during structural handling.
                continue
            for scope in matches:
                if scope.get('status') == 'needs_review':
                    scope['status'] = 'reviewed'
                if verdict.get('note'):
                    scope['note'] = verdict['note']
    return stale
