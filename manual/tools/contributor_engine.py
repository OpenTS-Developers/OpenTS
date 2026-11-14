"""Programmer-facing setup diagnostics and safe authoring scaffolds."""

import importlib.metadata
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys

from io_utils import atomic_write_text, load_yaml


ROOT = Path(__file__).resolve().parents[2]
MANUAL = ROOT / "manual"
TOOLS = MANUAL / "tools"
DATA = MANUAL / "data"
SITE = MANUAL / "site"


def _version_output(executable, argument="--version"):
    try:
        result = subprocess.run(
            [executable, argument], check=True, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    except (OSError, subprocess.CalledProcessError):
        return None
    return result.stdout.strip().lstrip("v")


def _required_packages():
    rows = []
    for raw in (TOOLS / "requirements.txt").read_text(encoding="utf-8").splitlines():
        text = raw.strip()
        if not text or text.startswith("#"):
            continue
        match = re.fullmatch(r"([A-Za-z0-9_.-]+)==([^\s]+)", text)
        if match:
            rows.append(match.groups())
    return rows


def doctor(verbose=False):
    """Return (verified messages, actions) for the complete local toolchain."""

    verified, actions = [], []
    python_file = TOOLS / ".python-version"
    if python_file.is_file():
        expected = python_file.read_text(encoding="utf-8").strip()
        actual = platform.python_version()
        if actual == expected:
            verified.append(f"Python {actual}")
        else:
            actions.append(
                f"Python {expected} is required (found {actual}). Install it and rerun with that interpreter.")
        if verbose:
            verified.append(f"Python executable: {sys.executable}")
            verified.append(f"Python version authority: {python_file.relative_to(ROOT)}")
    else:
        actions.append("Python version authority is missing: manual/tools/.python-version")

    for package, expected in _required_packages():
        try:
            actual = importlib.metadata.version(package)
        except importlib.metadata.PackageNotFoundError:
            actions.append(
                f"Python package {package}=={expected} is missing. Run: "
                "python -m pip install -r manual/tools/requirements.txt")
            continue
        if actual == expected:
            verified.append(f"Python package {package} {actual}")
        else:
            actions.append(
                f"Python package {package}=={expected} is required (found {actual}). Run: "
                "python -m pip install -r manual/tools/requirements.txt")

    package = json.loads((SITE / "package.json").read_text(encoding="utf-8"))
    node_authority = (SITE / ".nvmrc").read_text(encoding="utf-8").strip()
    declared_node = str(package.get("engines", {}).get("node", ""))
    if declared_node != node_authority:
        actions.append(
            f"manual/site/package.json engines.node ({declared_node!r}) must match "
            f"manual/site/.nvmrc ({node_authority!r}).")

    node = shutil.which("node")
    if not node:
        actions.append(
            f"Node {node_authority} is missing. Install the version in manual/site/.nvmrc.")
    else:
        actual = _version_output(node)
        if actual == node_authority:
            verified.append(f"Node {actual}")
        else:
            actions.append(
                f"Node {node_authority} is required (found {actual or 'an unreadable version'}). "
                "Install the version in manual/site/.nvmrc.")
        if verbose:
            verified.append(f"Node executable: {node}")

    package_manager = str(package.get("packageManager", ""))
    expected_npm = package_manager.removeprefix("npm@")
    npm = shutil.which("npm")
    if not npm:
        actions.append(
            f"npm {expected_npm} is missing. Install the packageManager version from manual/site/package.json.")
    else:
        actual = _version_output(npm)
        if actual == expected_npm:
            verified.append(f"npm {actual}")
        else:
            actions.append(
                f"npm {expected_npm} is required (found {actual or 'an unreadable version'}). "
                f"Run: npm install --global npm@{expected_npm}")
        if verbose:
            verified.append(f"npm executable: {npm}")

    if (SITE / "node_modules").is_dir():
        verified.append("Site dependencies are installed")
    else:
        actions.append("Site dependencies are missing. Run: cd manual/site; npm ci")

    return verified, actions


def _slugify(text):
    return re.sub(r"^-|-+$", "", re.sub(r"[^a-z0-9]+", "-", str(text).lower()))


def _display_name(identifier):
    value = re.sub(r"Type$", "", identifier)
    value = re.sub(r"^(?:TACTION|TEVENT|TMISSION)_", "", value)
    value = value.replace("_", " ")
    value = re.sub(r"(?<=[a-z0-9])(?=[A-Z])", " ", value)
    return " ".join(word.capitalize() for word in value.split())


def _document(frontmatter, body):
    import yaml
    header = yaml.safe_dump(
        frontmatter, sort_keys=False, allow_unicode=True, width=100).rstrip()
    return f"---\n{header}\n---\n\n{body.rstrip()}\n"


def _write(path, frontmatter, body):
    if path.exists():
        raise ValueError(f"refusing to overwrite existing authored content: {path.relative_to(ROOT)}")
    atomic_write_text(path, _document(frontmatter, body))
    print(f"GENERATED  {path.relative_to(ROOT)}")
    print("ACTION REQUIRED")
    print("  - Replace every TODO with source-checked explanatory prose.")
    print("  - Verify with: python manual/tools/manage.py check")


def _read_yaml(path):
    with path.open(encoding="utf-8") as stream:
        return load_yaml(stream)


def scaffold_key(identifier, scope=None):
    import validate_manual
    keys = _read_yaml(DATA / "ini-keys.yaml")
    if identifier not in keys:
        raise ValueError(
            f"unknown generated key {identifier!r}; run manage.py update first")
    record = keys[identifier]
    scopes = validate_manual.scope_route_ids(record)
    if scope is not None and scope not in scopes:
        raise ValueError(
            f"unknown scope {scope!r} for {identifier}; choose: {', '.join(scopes)}")
    if len(scopes) > 1 and scope is None:
        commands = "\n".join(
            f"  python manual/tools/manage.py scaffold key {identifier} --scope {candidate}"
            for candidate in scopes)
        raise ValueError(
            f"{identifier} has multiple meanings; scaffold one scope at a time:\n{commands}")
    route = validate_manual.key_route(identifier, record)
    suffix = f"--{scope}" if scope else ""
    path = MANUAL / "content" / "keys" / f"{route}{suffix}.md"
    if scope:
        # Reference tables read only unscoped summaries; a scoped overlay
        # carries a label, which renders as its section heading.
        frontmatter = {
            "key": identifier,
            "scope": scope,
            "label": f"TODO: Capitalized scope label for {identifier}",
        }
    else:
        frontmatter = {
            "key": identifier,
            "summary": f"TODO: Explain what {identifier} controls.",
        }
    _write(path, frontmatter, f"TODO: Explain `{identifier}` with a practical example.")


def scaffold_scripting(identifier):
    import versioning
    data = _read_yaml(DATA / "scripting.yaml")
    matches = []
    for folder, (table, singular) in versioning.SCRIPTING_TYPES.items():
        for row in data.get(table, []):
            if row.get("id") == identifier:
                matches.append((folder, singular, row))
    if not matches:
        raise ValueError(
            f"unknown generated scripting ID {identifier!r}; run manage.py update first")
    if len(matches) != 1:
        raise ValueError(f"generated scripting ID {identifier!r} is ambiguous")
    folder, singular, row = matches[0]
    path = (MANUAL / "content" / "scripting" / folder
            / f"{_slugify(identifier)}.md")
    title = row.get("editor_name") or _display_name(identifier)
    summary = row.get("editor_description") or f"TODO: Explain {title}."
    _write(path, {
        "type": singular,
        "id": identifier,
        "title": title,
        "summary": summary,
    }, "TODO: Explain behavior, constraints, and a practical use.")


def scaffold_enum(identifier):
    import enum_drift
    adapter = enum_drift.ADAPTERS.get(identifier)
    if adapter is None:
        raise ValueError(
            f"{identifier!r} has no explicit enum source adapter; add and test the adapter first")

    def reader(path):
        return (ROOT / path).read_text(encoding="utf-8")

    values = []
    for row in enum_drift.expected_values(adapter, reader):
        values.append({
            **row,
            "meaning": f"TODO: Explain {row['constant']}.",
        })
    slug = _slugify(_display_name(identifier))
    title = _display_name(identifier)
    path = MANUAL / "content" / "enums" / f"{slug}.md"
    _write(path, {
        "enum_id": identifier,
        "slug": slug,
        "title": title,
        "summary": f"TODO: Explain the {title} values accepted by the engine.",
        "representation": adapter.representation,
        "bindings": {
            "key_value_types": [],
            "scripting_parameter_types": [],
        },
        "source_files": list(adapter.source_files),
        "values": values,
    }, f"TODO: Explain where `{identifier}` is used.")


def _development_release():
    releases = _read_yaml(DATA / "releases.yaml").get("releases", [])
    matches = [row["version"] for row in releases if row.get("status") == "development"]
    if len(matches) != 1:
        raise ValueError("release registry must contain exactly one development release")
    return matches[0]


RELEASE_NOTE_SECTIONS = (
    ("feature", "New features"),
    ("fix", "Bug fixes"),
    ("balance", "Balance changes"),
    ("performance", "Performance"),
    ("internal", "Internal changes"),
)


def release_notes(version):
    """Render the change records assigned to one release as Markdown."""
    import validate_manual
    records = []
    for path in sorted((MANUAL / "changes").glob("*.md")):
        data = validate_manual.frontmatter(path)
        if data.get("release") == version:
            records.append(data)
    if not records:
        raise ValueError(f"no change records target release {version!r}")

    lines = []
    breaking = sorted(
        (row for row in records if row.get("breaking")),
        key=lambda row: row["title"])
    if breaking:
        lines += ["### Breaking changes", ""]
        for row in breaking:
            lines.append(f"- {row['title']}")
            lines.extend(
                f"  - Migration: {step}" for step in row.get("migration", ()))
        lines.append("")
    for category, heading in RELEASE_NOTE_SECTIONS:
        rows = sorted(
            (row for row in records if row.get("category") == category),
            key=lambda row: row["title"])
        if not rows:
            continue
        lines += [f"### {heading}", ""]
        for row in rows:
            credit = ", ".join(row.get("credit") or ())
            lines.append(f"- {row['title']}" + (f" (by {credit})" if credit else ""))
        lines.append("")
    return "\n".join(lines).rstrip() + "\n"
