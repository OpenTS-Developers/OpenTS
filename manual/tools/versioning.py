"""Structured release, lifecycle, tombstone, and route-alias validation."""

from datetime import date
from pathlib import Path
import re

from semantic_version import Version
import yaml


CHANGE_CATEGORIES = {"feature", "fix", "balance", "internal", "performance"}
ENTITY_TYPES = {"key", "action", "event", "mission", "format", "enum", "system", "command"}
RELATION_TYPES = ENTITY_TYPES | {"guide", "using", "internal"}
LIFECYCLE_EFFECTS = {"added", "changed", "deprecated", "removed"}
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
CHANGE_FIELDS = {
    "title", "category", "release", "breaking", "migration", "targets", "credit"
}
TARGET_FIELDS = {"type", "id", "scope", "effect"}
REFERENCE_FIELDS = {"type", "id", "scope"}
TOMBSTONE_FIELDS = {
    "type", "id", "route", "search_aliases", "summary", "replacement"
}


def read_yaml(path):
    with Path(path).open(encoding="utf-8") as stream:
        return yaml.safe_load(stream)


def frontmatter(path, root):
    source = path.read_text(encoding="utf-8")
    match = re.match(r"^---\r?\n([\s\S]*?)\r?\n---", source)
    if not match:
        raise ValueError(f"{path.relative_to(root)}: missing YAML frontmatter")
    return yaml.safe_load(match.group(1)) or {}


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
        route += "-" + slugify(applies[0])
    return route


def validate_releases(errors, manual, root):
    data = read_yaml(manual / "data" / "releases.yaml")
    if not isinstance(data, dict) or set(data) != {"releases"}:
        errors.append("manual/data/releases.yaml must contain only a releases array")
        return {"rows": [], "by_version": {}, "development": None}

    rows = data.get("releases")
    if not isinstance(rows, list) or not rows:
        errors.append("release registry must contain at least one release")
        return {"rows": [], "by_version": {}, "development": None}

    normalized = []
    seen = set()
    development = []
    for position, row in enumerate(rows, start=1):
        context = f"release registry entry {position}"
        if not isinstance(row, dict):
            errors.append(f"{context}: expected a mapping")
            continue
        allowed = {"version", "status", "date"}
        unexpected = set(row) - allowed
        if unexpected:
            errors.append(f"{context}: unexpected fields {sorted(unexpected)}")
        version_text = row.get("version")
        parsed = semver(version_text, context, errors)
        if version_text in seen:
            errors.append(f"{context}: duplicate version {version_text}")
        seen.add(version_text)
        status = row.get("status")
        if status not in {"development", "released"}:
            errors.append(f"{context}: status must be development or released")
        release_date = row.get("date")
        if status == "released":
            if not isinstance(release_date, str):
                errors.append(f"{context}: released entries require an ISO date")
            else:
                try:
                    parsed_date = date.fromisoformat(release_date)
                    if parsed_date.isoformat() != release_date:
                        raise ValueError
                except ValueError:
                    errors.append(f"{context}: invalid ISO date {release_date!r}")
        elif "date" in row:
            errors.append(f"{context}: development entries cannot have a date")
        if status == "development":
            development.append((version_text, parsed))
        normalized.append({**row, "_parsed": parsed})

    if len(development) != 1:
        errors.append("release registry must contain exactly one development release")
        development_version = None
    else:
        development_version, development_parsed = development[0]
        parsed_versions = [row["_parsed"] for row in normalized if row["_parsed"]]
        if development_parsed and parsed_versions and development_parsed != max(parsed_versions):
            errors.append("the development release must be the highest SemVer entry")

    if development_version:
        cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
        development_parsed = next(
            (row["_parsed"] for row in normalized
             if row.get("version") == development_version),
            None,
        )
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
        "rows": normalized,
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
    """Return command records keyed by their stable, case-preserved IDs.

    Working-tree validation already supplies a flattened catalog, while Git
    snapshots retain the three generated command arrays. Lifecycle comparison
    accepts either representation so a missing historical dataset can remain a
    distinct ``None`` baseline rather than looking like an empty catalog.
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


def scope_ids(key_entry):
    result = []
    used = set()
    for scope in key_entry.get("scopes", []):
        base = slugify((scope.get("applies_to") or [scope.get("section", "global")])[0])
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
        errors.append("manual/data/tombstones.yaml must be an array")
        return []
    scripts = scripting_catalog(scripting)
    seen_targets = set()
    seen_routes = set()
    result = []
    for position, row in enumerate(rows, start=1):
        context = f"tombstone entry {position}"
        if not isinstance(row, dict):
            errors.append(f"{context}: expected a mapping")
            continue
        unexpected = set(row) - TOMBSTONE_FIELDS
        missing = {"type", "id", "route", "search_aliases", "summary"} - set(row)
        if unexpected:
            errors.append(f"{context}: unexpected fields {sorted(unexpected)}")
        if missing:
            errors.append(f"{context}: missing fields {sorted(missing)}")
        entity_type, entity_id = row.get("type"), row.get("id")
        if entity_type not in ENTITY_TYPES:
            errors.append(f"{context}: invalid entity type {entity_type!r}")
        if not isinstance(entity_id, str) or not entity_id:
            errors.append(f"{context}: id must be a non-empty string")
        identity = (entity_type, entity_id)
        if identity in seen_targets:
            errors.append(f"{context}: duplicate tombstone target {entity_type}:{entity_id}")
        seen_targets.add(identity)
        route = row.get("route")
        if (not isinstance(route, str) or not route.startswith("/")
                or not route.endswith("/")):
            errors.append(f"{context}: route must be an absolute trailing-slash path")
        elif route in seen_routes:
            errors.append(f"{context}: duplicate tombstone route {route}")
        seen_routes.add(route)
        expected_route = None
        expected_family = None
        if entity_type == "command":
            # A launch option is published under its own family and keeps a route_id
            # that the catalog no longer carries once the option is removed, so only
            # the family can be checked here. The removal delta pins the whole route.
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
        if not isinstance(row.get("search_aliases"), list) or not all(
                isinstance(alias, str) and alias for alias in row.get("search_aliases", [])):
            errors.append(f"{context}: search_aliases must be an array of strings")
        if not isinstance(row.get("summary"), str) or not row.get("summary", "").strip():
            errors.append(f"{context}: summary must be a non-empty string")
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
    if not isinstance(data, dict) or set(data) != set(SCRIPTING_TYPES):
        errors.append(
            "manual/data/scripting-route-aliases.yaml must contain "
            "actions, events, and missions mappings")
        return {}
    scripts = scripting_catalog(scripting)
    tombstone_ids = {(row.get("type"), row.get("id")) for row in tombstones}
    normalized = {}
    for plural, (_, singular) in SCRIPTING_TYPES.items():
        mapping = data.get(plural)
        if not isinstance(mapping, dict):
            errors.append(f"scripting aliases {plural}: expected a mapping")
            continue
        normalized[plural] = {}
        for raw_index, entity_id in mapping.items():
            index = str(raw_index)
            context = f"scripting alias {plural}/{index}"
            if not re.fullmatch(r"0|[1-9][0-9]*", index):
                errors.append(f"{context}: route index must be a canonical non-negative integer")
            if not isinstance(entity_id, str) or not entity_id:
                errors.append(f"{context}: target must be a stable engine ID")
            elif (entity_id not in scripts.get(singular, {})
                    and (singular, entity_id) not in tombstone_ids):
                errors.append(f"{context}: unknown target {singular}:{entity_id}")
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
            errors.append(f"{context}: frontmatter must be a mapping")
            continue
        unexpected = set(data) - CHANGE_FIELDS
        missing = {"title", "category", "release", "targets", "credit"} - set(data)
        if unexpected:
            errors.append(f"{context}: unexpected fields {sorted(unexpected)}")
        if missing:
            errors.append(f"{context}: missing fields {sorted(missing)}")
        if not isinstance(data.get("title"), str) or not data.get("title", "").strip():
            errors.append(f"{context}: title must be a non-empty string")
        if data.get("category") not in CHANGE_CATEGORIES:
            errors.append(f"{context}: invalid category {data.get('category')!r}")
        release = data.get("release")
        if release not in versions:
            errors.append(f"{context}: unknown release {release!r}")
        breaking = data.get("breaking", False)
        if not isinstance(breaking, bool):
            errors.append(f"{context}: breaking must be a boolean")
            breaking = False
        migration = data.get("migration")
        if breaking:
            if (not isinstance(migration, list) or not migration
                    or not all(isinstance(step, str) and step.strip()
                               for step in migration)):
                errors.append(
                    f"{context}: breaking changes require a non-empty migration array")
        elif "migration" in data:
            errors.append(
                f"{context}: migration is allowed only when breaking is true")
        targets = data.get("targets")
        if not isinstance(targets, list):
            errors.append(f"{context}: targets must be an array")
            targets = []
        normalized_targets = []
        for position, raw in enumerate(targets, start=1):
            target_context = f"{context} target {position}"
            if not isinstance(raw, dict):
                errors.append(f"{target_context}: expected a mapping")
                continue
            unexpected_target = set(raw) - TARGET_FIELDS
            missing_target = {"type", "id", "effect"} - set(raw)
            if unexpected_target:
                errors.append(
                    f"{target_context}: unexpected fields {sorted(unexpected_target)}")
            if missing_target:
                errors.append(f"{target_context}: missing fields {sorted(missing_target)}")
            target = normalize_target(raw)
            if target["effect"] not in LIFECYCLE_EFFECTS:
                errors.append(
                    f"{target_context}: invalid lifecycle effect {target['effect']!r}")
            if target["scope"] is not None and target["type"] != "key":
                errors.append(f"{target_context}: scope is allowed only for key targets")
            if target["type"] not in ENTITY_TYPES:
                errors.append(f"{target_context}: invalid entity type {target['type']!r}")
            if not isinstance(target["id"], str) or not target["id"]:
                errors.append(f"{target_context}: id must be a non-empty string")
            elif target["effect"] == "removed":
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
                # History may cite an entity that was removed later; the lifecycle
                # ordering check keeps such events ahead of the removal.
                errors.append(
                    f"{target_context}: unknown active entity "
                    f"{target_description(target)}")
            normalized_targets.append(target)
        data["targets"] = normalized_targets
        credits = data.get("credit")
        if (not isinstance(credits, list) or not credits or not all(
                isinstance(value, str) and value for value in credits)):
            errors.append(
                f"{context}: credit must name at least one author "
                "as a non-empty array of strings")

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
                for field in ("category", "targets", "breaking", "migration"):
                    default = False if field == "breaking" else [] if field == "migration" else None
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


def key_scope_sets(keys):
    return {key: set(scope_ids(entry)) for key, entry in keys.items()}


def validate_branch_lifecycle(
        errors, current_keys, base_keys, current_scripting, base_scripting,
        current_formats, base_formats, changes, tombstones, key_delta,
        scripting_delta, enum_delta=None, base_enums=None,
        command_delta=None, base_commands=None):
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
            require_target(errors, changes, singular, entity_id, "changed")

    current_format_ids = set(current_formats)
    base_format_ids = set(base_formats or {})
    for entity_id in sorted(current_format_ids - base_format_ids):
        require_target(errors, changes, "format", entity_id, "added")
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
        # The enum catalog is a curated documentation selection: adding or
        # removing a page is a docs decision, not an engine lifecycle event.
        # Only signature drift on a selected domain reflects the engine.
        for entity_id in sorted(enum_delta["changed"]):
            require_target(errors, changes, "enum", entity_id, "changed")

    if command_delta is not None:
        base_command_records = command_catalog(base_commands)
        for entity_id in sorted(command_delta["added"]):
            require_target(errors, changes, "command", entity_id, "added")
        for entity_id in sorted(command_delta["changed"]):
            require_target(errors, changes, "command", entity_id, "changed")
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

