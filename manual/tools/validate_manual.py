"""Canonical manual validation facade.

The established lifecycle checks remain in :mod:`validate_manual_engine`.
This layer applies the shared JSON Schema contracts, stable authored filenames,
case-collision invariants, scaffold-marker checks, and compatibility handling
for the new public ``level`` field.
"""

import copy
import json
from pathlib import Path
import re

import validate_manual_engine as _engine
from validate_manual_engine import *  # noqa: F401,F403 - stable helper API

import schema_validation
import section_selectors


MANUAL = Path(__file__).resolve().parents[1]
SITE = MANUAL / "site"

SEMANTIC_SCOPE_FIELDS = (
    "applies_to", "file", "section", "value_type", "level", "note",
    "precedence", "read_from",
)


def _semantic_scope(scope):
    normalized = {
        field: scope[field]
        for field in SEMANTIC_SCOPE_FIELDS
        if field in scope
    }
    # Before this contract existed, the same fact was private provenance.  The
    # fallback prevents the migration itself from looking like 1,000 behavior
    # changes while still making future public level changes semantic.
    if "level" not in normalized:
        declared = (scope.get("_provenance") or {}).get("declared_in")
        if declared:
            normalized["level"] = declared
    return section_selectors.normalize_scope(normalized)


def semantic_key(entry):
    scopes = [_semantic_scope(scope) for scope in entry.get("scopes", [])]
    return sorted(
        json.dumps(scope, sort_keys=True, separators=(",", ":"))
        for scope in scopes)


_engine.semantic_key = semantic_key
_engine.SEMANTIC_SCOPE_FIELDS = SEMANTIC_SCOPE_FIELDS


DATA_CONTRACTS = (
    ("ini-keys.yaml", "generated-ini-keys.schema.json"),
    ("scripting.yaml", "generated-scripting.schema.json"),
    ("commands.yaml", "generated-commands.schema.json"),
    ("command-adapters.yaml", "command-adapters.schema.json"),
    ("releases.yaml", "releases.schema.json"),
    ("scripting-route-aliases.yaml", "scripting-route-aliases.schema.json"),
    ("tombstones.yaml", "tombstones.schema.json"),
    ("adjudications.yaml", "adjudications.schema.json"),
    ("ini-read-exclusions.yaml", "ini-read-exclusions.schema.json"),
)

AUTHORED_CONTRACTS = (
    ("content/keys", "*.md", "authored-key.schema.json"),
    ("content/scripting", "**/*.md", "authored-scripting.schema.json"),
    ("content/commands", "*.md", "authored-command.schema.json"),
    ("content/enums", "*.md", "authored-enum.schema.json"),
    ("content/formats", "*.md", "authored-format.schema.json"),
    ("content/guides", "*.md", "authored-guide.schema.json"),
    ("content/systems", "*.md", "authored-system.schema.json"),
    ("content/using", "*.md", "authored-using.schema.json"),
    ("content/internals", "*.md", "authored-internal.schema.json"),
    ("changes", "*.md", "authored-change.schema.json"),
    ("changes/rc", "*.md", "authored-rc-note.schema.json"),
)


def validate_contracts(errors):
    for filename, schema_name in DATA_CONTRACTS:
        path = MANUAL / "data" / filename
        if not path.is_file():
            errors.append(f"manual/data/{filename}: required contract input is missing")
            continue
        try:
            data = _engine.read_yaml(path)
        except (OSError, ValueError) as error:
            errors.append(f"manual/data/{filename}: cannot parse YAML: {error}")
            continue
        errors.extend(schema_validation.errors_for(
            data, schema_name, f"manual/data/{filename}"))

    for folder, pattern, schema_name in AUTHORED_CONTRACTS:
        for path in sorted((MANUAL / folder).glob(pattern)):
            try:
                data = _engine.frontmatter(path)
            except (OSError, ValueError) as error:
                errors.append(str(error))
                continue
            errors.extend(schema_validation.errors_for(
                data, schema_name, str(path.relative_to(MANUAL))))


def validate_section_selectors(errors, keys):
    """Give malformed generated selectors a concise, source-level diagnostic."""

    def check(value, context):
        try:
            section_selectors.require(value)
        except section_selectors.SectionSelectorError as error:
            errors.append(f"{context}: {error}")

    for name, entry in keys.items():
        if not isinstance(entry, dict):
            continue
        for index, scope in enumerate(entry.get("scopes", []), start=1):
            if not isinstance(scope, dict):
                continue
            if "section" in scope:
                check(scope["section"], f"{name} scope {index} section")
            read_sources = scope.get("read_from")
            if not isinstance(read_sources, list):
                continue
            for source_index, source in enumerate(read_sources, start=1):
                if isinstance(source, dict) and "section" in source:
                    check(
                        source["section"],
                        f"{name} scope {index} read_from {source_index} section",
                    )


def validate_case_collisions(errors, keys):
    for name, entry in keys.items():
        references = entry.get("case_collides_with") or []
        for target in references:
            if target not in keys:
                errors.append(
                    f"{name}: case_collides_with target {target!r} is not extracted")
                continue
            if target == name or target.lower() != name.lower():
                errors.append(
                    f"{name}: {target!r} is not a distinct case-only collision")
            reciprocal = keys[target].get("case_collides_with") or []
            if name not in reciprocal:
                errors.append(
                    f"{name}/{target}: case-collision links must be reciprocal")

    by_folded = {}
    for name in keys:
        by_folded.setdefault(name.lower(), []).append(name)
    for names in by_folded.values():
        if len(names) < 2:
            continue
        expected = set(names)
        for name in names:
            actual = set(keys[name].get("case_collides_with") or []) | {name}
            if actual != expected:
                errors.append(
                    f"{name}: case_collides_with must name exactly "
                    f"{sorted(expected - {name})}")


def validate_key_filenames(errors, keys):
    seen = {}
    for path in sorted((MANUAL / "content" / "keys").glob("*.md")):
        try:
            data = _engine.frontmatter(path)
        except (OSError, ValueError):
            continue
        name = data.get("key")
        if name not in keys:
            continue
        scope = data.get("scope")
        route = _engine.key_route(name, keys[name])
        expected = f"{route}{f'--{scope}' if scope else ''}.md"
        if path.name != expected:
            command = f"python manual/tools/manage.py scaffold key {name}"
            if scope:
                command += f" --scope {scope}"
            errors.append(
                f"{path.relative_to(MANUAL)}: expected filename "
                f"content/keys/{expected}; create the canonical scaffold with: {command}")
        identity = (name, scope)
        if identity in seen:
            errors.append(
                f"{path.relative_to(MANUAL)}: duplicate authored key/scope; "
                f"already provided by {seen[identity].relative_to(MANUAL)}")
        else:
            seen[identity] = path


def validate_placeholders(errors):
    marker = re.compile(r"(?m)^.*\bTODO:\s*.*$")
    for folder in (MANUAL / "content", MANUAL / "changes"):
        for path in sorted(folder.glob("**/*.md")):
            source = path.read_text(encoding="utf-8")
            match = marker.search(source)
            if match:
                errors.append(
                    f"{path.relative_to(MANUAL)}: unfinished scaffold marker "
                    f"{match.group(0).strip()!r}; replace it with source-checked prose")


def _with_extraction_corrections(base, current):
    if not base or not base.get("extraction_corrections"):
        return base
    effective = copy.deepcopy(base)
    effective.setdefault("keys", {})
    for key in effective["extraction_corrections"]:
        if key in current:
            effective["keys"][key] = current[key]
    return effective


def _actionable(error):
    omission = re.match(
        r"^([^\[]+)\[([^\]]+)\]: authored when_omitted metadata is required",
        error)
    if omission:
        key, scope = omission.groups()
        return (
            f"{error}; run: python manual/tools/manage.py scaffold key "
            f"{key} --scope {scope}")

    lifecycle = re.match(
        r"^(key|action|event|mission|format|enum|system|command):([^\[:]+)"
        r"(?:\[([^\]]+)\])?: add a change target with effect: ([a-z]+)",
        error)
    if lifecycle:
        entity_type, identifier, scope, effect = lifecycle.groups()
        change_id = _engine.slugify(f"{effect}-{identifier}")
        command = (
            f"python manual/tools/manage.py scaffold change {change_id} "
            f"--target-type {entity_type} --target-id {identifier} --effect {effect}")
        if scope:
            command += f" --scope {scope}"
        return f"{error}; run: {command}"
    return error


def validate_all(base=None):
    contract_errors = []
    validate_contracts(contract_errors)

    try:
        current = _engine.read_yaml(MANUAL / "data" / "ini-keys.yaml") or {}
    except (OSError, ValueError):
        current = {}
    effective_base = _with_extraction_corrections(base, current)
    core_errors, summary = _engine.validate_all(effective_base)

    extra_errors = []
    validate_section_selectors(extra_errors, current)
    validate_case_collisions(extra_errors, current)
    validate_key_filenames(extra_errors, current)
    validate_placeholders(extra_errors)

    errors = contract_errors + core_errors + extra_errors
    return [_actionable(error) for error in errors], summary
