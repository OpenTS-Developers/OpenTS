"""Structured release, lifecycle, tombstone, and route-alias validation.

The JSON schemas report malformed data; these checks skip what a schema rejects.
"""

from datetime import date
from pathlib import Path
import re

from semantic_version import Version

from io_utils import load_yaml


ENTITY_TYPES = {"key", "action", "event", "mission", "format", "enum", "system", "command"}
RELATION_TYPES = ENTITY_TYPES | {"guide", "using", "internal"}
SCRIPTING_TYPES = {
    "actions": ("trigger_actions", "action"),
    "events": ("trigger_events", "event"),
    "missions": ("team_missions", "mission"),
}
FORMAT_ROUTES = {
    "ai_triggers": "/mapping/ai-triggers/",
    "teamtypes": "/mapping/team-types/",
    "taskforces": "/mapping/task-forces/",
    "scripts": "/mapping/scripts/",
}
COMMAND_TABLES = ("registered_commands", "fixed_controls", "launch_options")
REFERENCE_FIELDS = {"type", "id", "scope"}


def read_yaml(path):
    with Path(path).open(encoding="utf-8") as stream:
        return load_yaml(stream)


def frontmatter(path, root):
    source = path.read_text(encoding="utf-8")
    match = re.match(r"^---\r?\n([\s\S]*?)\r?\n---", source)
    if not match:
        raise ValueError(f"{path.relative_to(root)}: missing YAML frontmatter")
    return load_yaml(match.group(1)) or {}


def slugify(text):
    return re.sub(r"^-|-+$", "", re.sub(r"[^a-z0-9]+", "-", str(text).lower()))


def semver(value, context, errors):
    if not isinstance(value, str) or not value:
        errors.append(f"{context}: version must be a non-empty SemVer string")
        return None
    if "+" in value:
        errors.append(f"{context}: build metadata is not allowed in release versions")
        return None
    try:
        return Version(value)
    except ValueError:
        errors.append(f"{context}: invalid SemVer version {value!r}")
        return None


def key_route(name, entry):
    route = name.lower()
    if entry.get("case_collides_with"):
        applies = entry.get("scopes", [{}])[0].get("applies_to") or ["global"]
        route += "-" + slugify(applies[0] or "global")
    return route


def validate_releases(errors, manual, root):
    data = read_yaml(manual / "data" / "releases.yaml")
    rows = data.get("releases") if isinstance(data, dict) else None
    if not isinstance(rows, list):
        return {"by_version": {}, "development": None}

    normalized = []
    seen = set()
    development = []
    for position, row in enumerate(rows, start=1):
        if not isinstance(row, dict):
            continue
        context = f"release registry entry {position}"
        version_text = row.get("version")
        parsed = semver(version_text, context, errors)
        if version_text in seen:
            errors.append(f"{context}: duplicate version {version_text}")
        seen.add(version_text)
        release_date = row.get("date")
        if isinstance(release_date, str):
            try:
                if date.fromisoformat(release_date).isoformat() != release_date:
                    raise ValueError
            except ValueError:
                errors.append(f"{context}: invalid ISO date {release_date!r}")
        if row.get("status") == "development":
            development.append((version_text, parsed))
        normalized.append({**row, "_parsed": parsed})

    development_version = None
    development_parsed = None
    if len(development) != 1:
        errors.append("release registry must contain exactly one development release")
    else:
        development_version, development_parsed = development[0]
        parsed_versions = [row["_parsed"] for row in normalized if row["_parsed"]]
        if development_parsed and parsed_versions and development_parsed != max(parsed_versions):
            errors.append("the development release must be the highest SemVer entry")

    if development_version:
        cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
        match = re.search(
            r"project\s*\(\s*OpenTS\s+VERSION\s+([0-9]+(?:\.[0-9]+){2,3})",
            cmake,
            re.IGNORECASE,
        )
        if not match:
            errors.append("CMakeLists.txt must declare project(OpenTS VERSION ...)")
        elif development_parsed:
            expected = (f"{development_parsed.major}.{development_parsed.minor}"
                        f".{development_parsed.patch}")
            actual_parts = match.group(1).split(".")
            actual = ".".join(actual_parts[:3])
            if actual != expected:
                errors.append(
                    "CMake project version must match the development "
                    f"SemVer core {expected}, found {match.group(1)}")

        # project() carries numbers only, so a prerelease label is declared beside it.
        if development_parsed:
            label = ".".join(development_parsed.prerelease)
            label_match = re.search(
                r"set\s*\(\s*OPENTS_VERSION_PRERELEASE\s+\"([^\"]*)\"\s*\)",
                cmake,
            )
            if not label_match:
                errors.append(
                    'CMakeLists.txt must declare set(OPENTS_VERSION_PRERELEASE "...")')
            elif label_match.group(1) != label:
                errors.append(
                    "CMake prerelease label must match the development version's "
                    f"label {label!r}, found {label_match.group(1)!r}")

    by_version = {
        row["version"]: row for row in normalized
        if isinstance(row.get("version"), str)
    }
    return {
        "by_version": by_version,
        "development": development_version,
    }


def load_changes(errors, manual):
    folder = manual / "changes"
    changes = {}
    for path in folder.rglob("*.md"):
        if path.parent != folder:
            errors.append(
                f"{path.relative_to(manual)}: change fragments must be flat; "
                "nested change folders are not supported")
            continue
        change_id = path.stem
        if not re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", change_id):
            errors.append(
                f"{path.relative_to(manual)}: filename must be a stable kebab-case change ID")
        if change_id in changes:
            errors.append(f"duplicate change ID: {change_id}")
            continue
        try:
            data = frontmatter(path, manual)
        except ValueError as error:
            errors.append(str(error))
            continue
        changes[change_id] = {"id": change_id, "path": path, "data": data}
    return changes


def normalize_target(raw):
    return {
        "type": raw.get("type"),
        "id": raw.get("id"),
        "scope": raw.get("scope"),
        "effect": raw.get("effect"),
    }


def target_key(target):
    return (target.get("type"), target.get("id"), target.get("scope"))


def target_description(target):
    suffix = f"[{target['scope']}]" if target.get("scope") else ""
    return f"{target.get('type')}:{target.get('id')}{suffix}"


def scripting_catalog(scripting):
    result = {}
    for plural, (table, singular) in SCRIPTING_TYPES.items():
        result[singular] = {
            row.get("id"): row for row in scripting.get(table, [])
            if isinstance(row, dict) and row.get("id")
        }
    return result


def command_catalog(commands):
    """Return command records keyed by ID.

    *commands* is either the generated file's three arrays or an already
    flattened mapping; any other value gives an empty catalog.
    """
    if not isinstance(commands, dict):
        return {}
    if any(table in commands for table in COMMAND_TABLES):
        result = {}
        for table in COMMAND_TABLES:
            for row in commands.get(table, []):
                if isinstance(row, dict) and isinstance(row.get("id"), str):
                    result[row["id"]] = row
        return result
    return {
        entity_id: row for entity_id, row in commands.items()
        if isinstance(entity_id, str) and isinstance(row, dict)
    }


def scope_route_base(scope):
    return slugify((scope.get("applies_to") or [scope.get("section", "global")])[0])


def scope_ids(key_entry):
    result = []
    used = set()
    for scope in key_entry.get("scopes", []):
        base = scope_route_base(scope)
        candidate = base
        suffix = 2
        while candidate in used:
            candidate = f"{base}-{suffix}"
            suffix += 1
        used.add(candidate)
        result.append(candidate)
    return result


def active_entity(target, keys, scripts, formats, enums=None, entities=None):
    entity_type, entity_id, scope = target_key(target)
    if entity_type == "key":
        entry = keys.get(entity_id)
        if not entry:
            return False
        return scope is None or scope in scope_ids(entry)
    if entity_type in scripts:
        return scope is None and entity_id in scripts[entity_type]
    if entity_type == "format":
        return scope is None and entity_id in formats
    if entity_type == "enum":
        return scope is None and entity_id in (enums or {})
    if entity_type in (entities or {}):
        return scope is None and entity_id in entities[entity_type]
    return False


def entity_route(entity_type, entity_id, record=None):
    if entity_type == "key":
        return f"/keys/{key_route(entity_id, record or {})}/"
    if entity_type in {"action", "event", "mission"}:
        return f"/mapping/{entity_type}s/{slugify(entity_id)}/"
    if isinstance(record, dict) and record.get("route"):
        return record["route"]
    if entity_type == "format":
        return FORMAT_ROUTES.get(entity_id, f"/formats/{slugify(entity_id)}/")
    if entity_type == "enum" and isinstance(record, dict) and record.get("slug"):
        return f"/reference/enums/{record['slug']}/"
    if entity_type == "command":
        route_id = record.get("route_id") if isinstance(record, dict) else None
        kind = record.get("kind") if isinstance(record, dict) else None
        family = "/using/command-line" if kind == "launch" else "/commands"
        return f"{family}/{route_id or slugify(entity_id)}/"
    prefix = {
        "system": "systems", "guide": "guides", "using": "using",
        "internal": "internals",
    }.get(entity_type)
    if prefix:
        return f"/{prefix}/{slugify(entity_id)}/"
    return None


def validate_reference(errors, raw, context, keys, scripts, formats,
                       tombstones=None, enums=None, entities=None, allowed_types=None):
    if not isinstance(raw, dict):
        errors.append(f"{context}: expected a typed entity reference")
        return False
    unexpected = set(raw) - REFERENCE_FIELDS
    if unexpected:
        errors.append(f"{context}: unexpected fields {sorted(unexpected)}")
    entity_type, entity_id, scope = raw.get("type"), raw.get("id"), raw.get("scope")
    allowed = RELATION_TYPES if allowed_types is None else allowed_types
    if entity_type not in allowed:
        errors.append(f"{context}: invalid entity type {entity_type!r}")
        return False
    if not isinstance(entity_id, str) or not entity_id:
        errors.append(f"{context}: id must be a non-empty string")
        return False
    if scope is not None and entity_type != "key":
        errors.append(f"{context}: scope is allowed only for key references")
        return False
    target = {"type": entity_type, "id": entity_id, "scope": scope}
    if active_entity(target, keys, scripts, formats, enums, entities):
        return True
    if tombstones and (entity_type, entity_id) in tombstones and scope is None:
        return True
    errors.append(f"{context}: unknown entity {target_description(target)}")
    return False


def validate_tombstones(
        errors, manual, keys, scripting, formats, enums=None, entities=None):
    rows = read_yaml(manual / "data" / "tombstones.yaml") or []
    if not isinstance(rows, list):
        return []
    scripts = scripting_catalog(scripting)
    seen_targets = set()
    seen_routes = set()
    result = []
    for position, row in enumerate(rows, start=1):
        if not isinstance(row, dict):
            continue
        context = f"tombstone entry {position}"
        entity_type, entity_id = row.get("type"), row.get("id")
        if not isinstance(entity_id, str) or not entity_id:
            continue
        identity = (entity_type, entity_id)
        if identity in seen_targets:
            errors.append(f"{context}: duplicate tombstone target {entity_type}:{entity_id}")
        seen_targets.add(identity)
        route = row.get("route")
        if route in seen_routes:
            errors.append(f"{context}: duplicate tombstone route {route}")
        seen_routes.add(route)
        expected_route = None
        expected_family = None
        if entity_type == "command":
            # A removed launch option's route_id is gone from the catalog, so only the
            # route prefix is checked here; validate_branch_lifecycle checks the full route.
            if entity_id.startswith("launch:"):
                expected_family = "/using/command-line/"
            else:
                expected_route = entity_route(entity_type, entity_id)
        elif entity_type == "system":
            expected_route = entity_route(entity_type, entity_id)
        elif entity_type == "format" and entity_id in FORMAT_ROUTES:
            expected_route = entity_route(entity_type, entity_id)
        if expected_route and route != expected_route:
            errors.append(
                f"{context}: {entity_type} route must remain {expected_route!r}, "
                f"found {route!r}")
        if expected_family and isinstance(route, str) and not route.startswith(expected_family):
            errors.append(
                f"{context}: {entity_type} route must stay under {expected_family!r}, "
                f"found {route!r}")
        target = {"type": entity_type, "id": entity_id, "scope": None}
        if active_entity(target, keys, scripts, formats, enums, entities):
            errors.append(f"{context}: tombstone target is still active")
        replacement = row.get("replacement")
        if replacement is not None:
            validate_reference(
                errors, replacement, f"{context} replacement",
                keys, scripts, formats, enums=enums, entities=entities,
                allowed_types=ENTITY_TYPES)
        result.append(row)

    active_routes = {f"/keys/{key_route(name, entry)}/" for name, entry in keys.items()}
    for singular, records in scripts.items():
        active_routes.update(
            entity_route(singular, entity_id) for entity_id in records)
    active_routes.update(
        entity_route("format", entity_id, record)
        for entity_id, record in formats.items())
    active_routes.update(
        entity_route("enum", entity_id, record)
        for entity_id, record in (enums or {}).items())
    for entity_type in ENTITY_TYPES & set(entities or {}):
        active_routes.update(
            entity_route(entity_type, entity_id, record)
            for entity_id, record in entities[entity_type].items())
    for row in result:
        if row.get("route") in active_routes:
            errors.append(
                f"tombstone {row.get('type')}:{row.get('id')}: "
                f"route collides with an active entity: {row.get('route')}")
    return result


def validate_aliases(errors, manual, scripting, tombstones, base_aliases=None):
    data = read_yaml(manual / "data" / "scripting-route-aliases.yaml")
    if not isinstance(data, dict):
        return {}
    scripts = scripting_catalog(scripting)
    tombstone_ids = {(row.get("type"), row.get("id")) for row in tombstones}
    normalized = {}
    for plural, (_, singular) in SCRIPTING_TYPES.items():
        mapping = data.get(plural)
        if not isinstance(mapping, dict):
            continue
        normalized[plural] = {}
        for raw_index, entity_id in mapping.items():
            index = str(raw_index)
            if (entity_id not in scripts.get(singular, {})
                    and (singular, entity_id) not in tombstone_ids):
                errors.append(
                    f"scripting alias {plural}/{index}: unknown target {singular}:{entity_id}")
            normalized[plural][index] = entity_id

        for entity_id, row in scripts.get(singular, {}).items():
            current_index = str(row.get("index"))
            if current_index not in normalized[plural]:
                errors.append(
                    f"{singular}:{entity_id}: reserve unused numeric route "
                    f"{plural}/{current_index} as a permanent alias")

    if isinstance(base_aliases, dict):
        for plural, mapping in base_aliases.items():
            if not isinstance(mapping, dict):
                continue
            current = normalized.get(plural, {})
            base_mapping = {
                str(raw_index): entity_id
                for raw_index, entity_id in mapping.items()
            }
            for index, entity_id in base_mapping.items():
                if current.get(index) != entity_id:
                    errors.append(
                        f"scripting alias {plural}/{index} is permanent and "
                        f"must remain assigned to {entity_id}")
            for index, entity_id in current.items():
                if index in base_mapping:
                    continue
                row = scripts.get(SCRIPTING_TYPES[plural][1], {}).get(entity_id)
                if not row or str(row.get("index")) != index:
                    errors.append(
                        f"new scripting alias {plural}/{index} must use an "
                        "unreserved entity's current serialized index")
    return normalized


def validate_changes(
        errors, manual, registry, keys, scripting, formats, tombstones,
        base_changes=None, base_registry=None, enums=None, entities=None):
    changes = load_changes(errors, manual)
    scripts = scripting_catalog(scripting)
    tombstone_map = {
        (row.get("type"), row.get("id")): row for row in tombstones
    }
    versions = registry["by_version"]
    development = registry["development"]

    for change_id, change in changes.items():
        data = change["data"]
        context = f"changes/{change_id}.md"
        if change_id in versions:
            errors.append(
                f"{context}: change ID collides with release upgrade route /changes/{change_id}/")
        if not isinstance(data, dict):
            continue
        release = data.get("release")
        if release not in versions:
            errors.append(f"{context}: unknown release {release!r}")
        targets = data.get("targets")
        if not isinstance(targets, list):
            targets = []
        normalized_targets = []
        for position, raw in enumerate(targets, start=1):
            if not isinstance(raw, dict):
                continue
            target_context = f"{context} target {position}"
            target = normalize_target(raw)
            normalized_targets.append(target)
            if not isinstance(target["id"], str) or not target["id"]:
                continue
            if target["effect"] == "removed":
                if target["scope"] is None:
                    if (target["type"], target["id"]) not in tombstone_map:
                        errors.append(
                            f"{target_context}: removed entity requires a matching tombstone")
                elif target["type"] != "key" or target["id"] not in keys:
                    errors.append(
                        f"{target_context}: removed key scope requires an active parent key")
                elif target["scope"] in scope_ids(keys[target["id"]]):
                    errors.append(
                        f"{target_context}: removed key scope is still active")
            elif (not active_entity(target, keys, scripts, formats, enums, entities)
                    and (target["scope"] is not None
                         or (target["type"], target["id"]) not in tombstone_map)):
                # A target may name a removed entity that has a tombstone;
                # validate_history rejects events at or after its removal.
                errors.append(
                    f"{target_context}: unknown active entity "
                    f"{target_description(target)}")
        data["targets"] = normalized_targets

        base_change = (base_changes or {}).get(change_id)
        if base_registry and base_change is None and release != development:
            errors.append(
                f"{context}: new changes must target development release {development}")
        if base_change and base_registry:
            old_release = base_change.get("release")
            if release != old_release:
                errors.append(
                    f"{context}: release assignment is stable and cannot be "
                    f"changed from {old_release!r} to {release!r}")
            old_release_row = base_registry.get("by_version", {}).get(old_release)
            current_release_row = versions.get(old_release)
            if ((old_release_row and old_release_row.get("status") == "released")
                    or (current_release_row
                        and current_release_row.get("status") == "released")):
                for field in ("category", "targets", "breaking"):
                    default = False if field == "breaking" else None
                    base_value = base_change.get(field, default)
                    # Base snapshots carry raw frontmatter while the current
                    # targets were normalized above, so compare one form.
                    if field == "targets" and isinstance(base_value, list):
                        base_value = [
                            normalize_target(raw) for raw in base_value
                            if isinstance(raw, dict)
                        ]
                    if data.get(field, default) != base_value:
                        errors.append(
                            f"{context}: released lifecycle field {field} is immutable")

    if base_registry:
        for removed_id in sorted(set(base_changes or {}) - set(changes)):
            errors.append(
                f"changes/{removed_id}.md: stable change IDs cannot be removed "
                "or renamed")

    return changes


def validate_history(errors, registry, changes, tombstones):
    history = {}
    for change_id, change in changes.items():
        release = change["data"].get("release")
        parsed = registry["by_version"].get(release, {}).get("_parsed")
        for target in change["data"].get("targets", []):
            history.setdefault(target_key(target), []).append({
                "change": change_id,
                "release": release,
                "parsed": parsed,
                "effect": target.get("effect"),
                "target": target,
            })

    for identity, events in history.items():
        additions = [event for event in events if event["effect"] == "added"]
        removals = [event for event in events if event["effect"] == "removed"]
        label = target_description({
            "type": identity[0], "id": identity[1], "scope": identity[2]
        })
        if len(additions) > 1:
            errors.append(f"{label}: lifecycle may contain at most one added event")
        if len(removals) > 1:
            errors.append(f"{label}: lifecycle may contain at most one removed event")
        comparable = [event for event in events if event["parsed"]]
        if additions and additions[0]["parsed"]:
            for event in comparable:
                if event["parsed"] < additions[0]["parsed"]:
                    errors.append(f"{label}: lifecycle event precedes its addition")
        if removals and removals[0]["parsed"]:
            removal = removals[0]
            for event in comparable:
                if event is not removal and event["parsed"] >= removal["parsed"]:
                    errors.append(f"{label}: lifecycle event follows its removal")

    for row in tombstones:
        identity = (row.get("type"), row.get("id"), None)
        removals = [
            event for event in history.get(identity, [])
            if event["effect"] == "removed"
        ]
        if len(removals) != 1:
            errors.append(
                f"tombstone {row.get('type')}:{row.get('id')} requires "
                "exactly one authoritative removed change target")
    return history


def has_target(changes, entity_type, entity_id, effect, scope=None):
    return any(
        target.get("type") == entity_type
        and target.get("id") == entity_id
        and target.get("effect") == effect
        and target.get("scope") == scope
        for change in changes.values()
        for target in change["data"].get("targets", [])
    )


def require_target(errors, changes, entity_type, entity_id, effect, scope=None):
    if not has_target(changes, entity_type, entity_id, effect, scope):
        suffix = f", scope: {scope}" if scope else ""
        errors.append(
            f"{entity_type}:{entity_id}{f'[{scope}]' if scope else ''}: "
            f"add a change target with effect: {effect}{suffix}")


def development_additions(changes, registry, base_changes, base_registry):
    development = (registry or {}).get("development")
    if not development or development != (base_registry or {}).get("development"):
        return set()
    for snapshot in (registry, base_registry):
        if snapshot.get("by_version", {}).get(development, {}).get("status") != "development":
            return set()

    current_additions = {
        target_key(target)
        for change in changes.values()
        if change["data"].get("release") == development
        for target in change["data"].get("targets", [])
        if target.get("effect") == "added"
    }
    base_additions = {
        target_key(target)
        for change in (base_changes or {}).values()
        if change.get("release") == development
        for target in change.get("targets", [])
        if target.get("effect") == "added"
    }
    return current_additions & base_additions


def require_changed_target(errors, changes, entity_type, entity_id, additions):
    if (entity_type, entity_id, None) not in additions:
        require_target(errors, changes, entity_type, entity_id, "changed")


def key_scope_sets(keys):
    return {key: set(scope_ids(entry)) for key, entry in keys.items()}


def validate_branch_lifecycle(
        errors, current_keys, base_keys, current_formats, base_formats,
        changes, tombstones, key_delta, scripting_delta, enum_delta=None,
        command_delta=None, base_commands=None, registry=None,
        base_changes=None, base_registry=None):
    additions = development_additions(changes, registry, base_changes, base_registry)
    tombstone_map = {
        (row.get("type"), row.get("id")): row for row in tombstones
    }
    for key in sorted(key_delta["added"]):
        require_target(errors, changes, "key", key, "added")
    for key in sorted(key_delta["removed"]):
        require_target(errors, changes, "key", key, "removed")
        record = tombstone_map.get(("key", key))
        expected = f"/keys/{key_route(key, base_keys[key])}/"
        if not record:
            errors.append(f"key:{key}: removed key requires a tombstone")
        elif record.get("route") != expected:
            errors.append(
                f"key:{key}: tombstone route must remain {expected!r}, "
                f"found {record.get('route')!r}")

    current_scopes = key_scope_sets(current_keys)
    base_scopes = key_scope_sets(base_keys)
    for key in sorted(set(current_keys) & set(base_keys)):
        for scope in sorted(current_scopes[key] - base_scopes[key]):
            require_target(errors, changes, "key", key, "added", scope)
        for scope in sorted(base_scopes[key] - current_scopes[key]):
            require_target(errors, changes, "key", key, "removed", scope)

    for plural, changes_for_kind in scripting_delta.items():
        singular = SCRIPTING_TYPES[plural][1]
        for entity_id in sorted(changes_for_kind["added"]):
            require_target(errors, changes, singular, entity_id, "added")
        for entity_id in sorted(changes_for_kind["removed"]):
            require_target(errors, changes, singular, entity_id, "removed")
        for entity_id in sorted(changes_for_kind["shifted"]):
            require_changed_target(errors, changes, singular, entity_id, additions)

    current_format_ids = set(current_formats)
    base_format_ids = set(base_formats or {})
    # A new format page needs no record, because most formats predate OpenTS. A removed
    # page needs a removal record and a tombstone so that its route still resolves.
    for entity_id in sorted(base_format_ids - current_format_ids):
        require_target(errors, changes, "format", entity_id, "removed")
        record = tombstone_map.get(("format", entity_id))
        expected = entity_route("format", entity_id, base_formats[entity_id])
        if not record:
            errors.append(f"format:{entity_id}: removed format requires a tombstone")
        elif record.get("route") != expected:
            errors.append(
                f"format:{entity_id}: tombstone route must remain {expected!r}, "
                f"found {record.get('route')!r}")
    if enum_delta:
        # Adding or removing an enum page needs no record; a change to a documented
        # enum's values does.
        for entity_id in sorted(enum_delta["changed"]):
            require_changed_target(errors, changes, "enum", entity_id, additions)

    if command_delta is not None:
        base_command_records = command_catalog(base_commands)
        for entity_id in sorted(command_delta["added"]):
            require_target(errors, changes, "command", entity_id, "added")
        for entity_id in sorted(command_delta["changed"]):
            require_changed_target(errors, changes, "command", entity_id, additions)
        for entity_id in sorted(command_delta["removed"]):
            require_target(errors, changes, "command", entity_id, "removed")
            record = tombstone_map.get(("command", entity_id))
            expected = entity_route(
                "command", entity_id, base_command_records.get(entity_id))
            if not record:
                errors.append(
                    f"command:{entity_id}: removed command requires a tombstone")
            elif record.get("route") != expected:
                errors.append(
                    f"command:{entity_id}: tombstone route must remain {expected!r}, "
                    f"found {record.get('route')!r}")


def validate_released_registry_history(errors, registry, base_registry):
    if not base_registry:
        return
    current = registry["by_version"]
    for version, row in base_registry.get("by_version", {}).items():
        if row.get("status") != "released":
            continue
        candidate = current.get(version)
        if not candidate:
            errors.append(f"released version {version} cannot be removed")
        elif candidate.get("status") != "released":
            errors.append(f"released version {version} cannot be demoted")
        elif candidate.get("date") != row.get("date"):
            errors.append(f"released version {version} date is immutable")

    old_development = base_registry.get("development")
    if old_development and old_development not in current:
        errors.append(
            f"development version {old_development} must remain in the registry "
            "and be marked released before opening the next development version")


def omission_targets(changes):
    result = set()
    for change in changes.values():
        for target in change["data"].get("targets", []):
            if (target.get("type") == "key"
                    and target.get("effect") in {"added", "changed"}):
                result.add((target.get("id"), target.get("scope")))
    return result

