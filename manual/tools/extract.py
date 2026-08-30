"""Extract the public INI catalog with contracts and atomic output safety.

The parsing engine lives in :mod:`extract_engine`; this entry point owns the
programmer-facing orchestration boundary: automatic reader discovery,
exception inventory, shared-contract validation, and output publication.
"""

import argparse
import json
import os
from pathlib import Path
import re
import sys

import extract_engine as _engine
from extract_engine import *  # noqa: F401,F403 - preserve extractor helper API

from io_utils import atomic_write_text
import schema_validation
import section_selectors


# Readers outside the type hierarchy need enough context to turn implementation
# variables into useful file/section/scope labels.  Ordinary Class::Read_INI
# methods not listed here are enrolled automatically by ini_inventory.
ADAPTER_UNITS = [
    ("base.cpp", "BaseClass", ["Read_INI"],
     {"file": "map file", "group": "scenario bases",
      "section_vars": {
          "hname": section_selectors.identifier("house"),
      }}),
    ("display.cpp", "DisplayClass", ["Read_INI"],
     {"file": "map file", "group": "scenarios",
      "section_vars": {
          "MAP_NAME": section_selectors.literal("Map"),
          "name": section_selectors.literal("Map"),
      }}),
    ("levitate.cpp", "LevitateLocomotionClass", ["Read_INI"],
     {"file": "rules.ini", "group": "levitation controls",
      "section_vars": {
          "INI_NAME": section_selectors.literal("LEVITATION"),
      }}),
    # MissionControlClass::Name() returns Missions[Mission] from code/_mission.cpp:
    # the mission's own name, which doubles as its rules section. It is not an
    # object type -- no object is involved in the read at all.
    ("mission.cpp", "MissionControlClass", ["Read_INI"],
     {"file": "rules.ini", "group": "mission behavior",
      "section_vars": {
          "Name()": section_selectors.identifier("mission"),
      }}),
    # Both MultiMission constructors are live and read the same three settings
    # from different places: SessionClass::Read_Scenario_Descriptions builds a
    # packet entry through the first, which reads the section the .PKT named,
    # and a loose map through the second, which reads [Multiplay] out of the
    # .MPR itself. Neither is covered by any Read_INI reader.
    ("session.cpp", "MultiMission", [":MultiMission(INIClass"],
     {"file": "scenario packet (.pkt)", "group": "map packets",
      "section_vars": {
          "name": section_selectors.identifier("multiplayer-map"),
      }}),
    ("session.cpp", "MultiMission", [":MultiMission(char const * filename"],
     {"file": "map file (.mpr)", "group": "multiplayer maps"}),
    ("isotype.cpp", "IsometricTileTypeClass", ["Read_Control_File"],
     {"file": "theater control file", "group": "tile controls",
      "section_vars": {
          "IniName": section_selectors.identifier("tile-set"),
          "SetName": section_selectors.identifier("tile-set"),
          "section": section_selectors.identifier("tile-set"),
      }}),
    ("mapgen.cpp", "MapSeedClass", ["Load_File", "Read_File"],
     {"file": "map seed file", "group": "random map generation",
      "section_vars": {
          "MAPSEED_SECTION": section_selectors.literal("MapSeed"),
      }}),
    ("mapgen.cpp", "MapGeneratorClass", ["Init_Map"],
     {"file": "map file", "group": "random map generation"}),
    ("netshare.cpp", "RandomMap", [":RandomMapWaypointCount"],
     {"file": "map seed file", "group": "random map generation"}),
    ("rules.cpp", "DifficultyClass", [":Difficulty_Get"],
     {"file": "rules.ini", "group": "difficulty settings",
      "section_vars": {
          "section": section_selectors.identifier("difficulty"),
      }}),
    ("rules.cpp", "RulesClass", ["Special_Weapons", "Land_Types"],
     {"file": "rules.ini", "group": "global rules",
      "section_vars": {
          "_lands[land]": section_selectors.identifier("land-type"),
      }}),
    ("builtype.cpp", "BuildingTypeClass", ["Fetch_Building_Normal_Image"],
     {"file": "art.ini", "group": "BuildingType",
      "section_vars": {
          "GraphicName": section_selectors.image(),
          "Graphic_Name()": section_selectors.image(),
      }}),
    ("infatype.cpp", "InfantryTypeClass", ["Read_Sequence_INI"],
     {"file": "art.ini", "group": "InfantryType",
      "section_vars": {
          "GraphicName": section_selectors.image(),
          "Graphic_Name()": section_selectors.image(),
      }}),
    ("startup.cpp", "Startup", [":WinMain"],
     {"file": "sun.ini", "group": "client settings"}),
]


def _body_at(text, brace):
    depth = 0
    index = brace
    while index < len(text):
        char = text[index]
        if char == '"':
            index += 1
            while index < len(text) and text[index] != '"':
                index += 2 if text[index] == "\\" else 1
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[brace:index + 1]
        index += 1
    return None


def find_body(text, header_re):
    """Choose a definition, never a forward declaration or wrong overload.

    A few legacy files declare a reader before defining it, and scenario.cpp
    contains overloads with the same spelling.  Ranking valid definitions by
    typed reads and extractor macros selects the implementation that carries
    the public data rather than whichever signature appears first.
    """

    candidates = []
    for match in header_re.finditer(text):
        brace = text.find("{", match.end())
        semicolon = text.find(";", match.end())
        if brace < 0 or (semicolon >= 0 and semicolon < brace):
            continue
        body = _body_at(text, brace)
        if body is None:
            continue
        score = len(list(_engine.GET_CALL_RE.finditer(body)))
        candidates.append((score, len(body), body, brace))
    if not candidates:
        return None, None
    _, _, body, brace = max(candidates, key=lambda row: (row[0], row[1]))
    return body, brace


# The engine functions resolve find_body through their defining module.
_engine.find_body = find_body


def _configured_globals():
    configured = list(_engine.GLOBAL_UNITS)
    identities = {(row[0], row[1], tuple(row[2])) for row in configured}
    for row in ADAPTER_UNITS:
        identity = (row[0], row[1], tuple(row[2]))
        if identity not in identities:
            configured.append(row)
    return configured


GLOBAL_UNITS = _configured_globals()
UNITS = list(_engine.UNITS)


def _extract_all(manifest):
    import ini_inventory

    tree = _engine.load_hierarchy()
    records_by_class = {}
    defaults_by_class = {}
    missing = []

    configured = set(UNITS)
    configured.update(
        (filename, cls)
        for filename, cls, methods, _ in GLOBAL_UNITS
        if "Read_INI" in methods)
    units = UNITS + ini_inventory.discover_read_ini_units(
        _engine.CODE_DIR, manifest, configured)

    for filename, cls in units:
        path = os.path.join(_engine.CODE_DIR, filename)
        if not os.path.exists(path):
            missing.append(filename)
            continue
        records, defaults = _engine.extract_file(path, cls, tree)
        key = cls if cls not in records_by_class else f"{cls}/{filename}"
        records_by_class[key] = records
        defaults_by_class[key] = defaults

    for filename, cls, methods, options in GLOBAL_UNITS:
        path = os.path.join(_engine.CODE_DIR, filename)
        if not os.path.exists(path):
            missing.append(filename)
            continue
        records, defaults = _engine.extract_globals(
            path, cls, methods, options)
        key = cls if cls not in records_by_class else f"{cls}/{methods[0]}"
        records_by_class[key] = records
        defaults_by_class[key] = defaults

    # An excluded read is kept out of the catalog even where a configured
    # reader owns its site, so one manifest governs the classification whether
    # or not the site happens to sit inside an enrolled unit.
    sites = ini_inventory.discover_literal_reads(_engine.CODE_DIR)
    records_by_class = ini_inventory.drop_suppressed(
        records_by_class, manifest, sites)

    inventory_errors, inventory_summary = ini_inventory.validate_inventory(
        records_by_class, manifest, _engine.CODE_DIR, sites=sites)
    if inventory_errors:
        raise ValueError("\n".join(inventory_errors))

    keys = _engine.build(records_by_class, defaults_by_class, tree)
    stale = _engine.apply_adjudications(keys, _engine.load_adjudications())
    if stale:
        raise ValueError("\n".join(
            f"stale adjudication: {warning}" for warning in stale))

    for entry in keys.values():
        for scope in entry.get("scopes", []):
            provenance = scope.get("_provenance") or {}
            scope["level"] = provenance.get("declared_in") or "global"

    contract_errors = schema_validation.errors_for(
        keys, "generated-ini-keys.schema.json", "generated INI catalog")
    if contract_errors:
        raise ValueError("\n".join(contract_errors))
    return keys, records_by_class, missing, inventory_summary


def _report(keys, records_by_class, missing, inventory):
    scopes = [scope for entry in keys.values() for scope in entry["scopes"]]
    multi = [entry for entry in keys.values() if len(entry["scopes"]) > 1]
    print(f"units scanned      : {len(records_by_class)}")
    if missing:
        print("units missing      : " + ", ".join(sorted(set(missing))))
    print(f"literal INI reads  : {inventory['sites']}")
    print(f"automatically owned: {inventory['extracted']}")
    print(
        "exceptional reads  : "
        f"{inventory['adapter']} adapter, {inventory['duplicate']} duplicate, "
        f"{inventory['excluded']} non-public")
    print(f"raw key reads      : {sum(len(rows) for rows in records_by_class.values())}")
    print(f"distinct keys      : {len(keys)}")
    print(f"documented scopes  : {len(scopes)}")
    print(f"multi-meaning keys : {len(multi)}")
    by_file = {}
    for scope in scopes:
        by_file[scope["file"]] = by_file.get(scope["file"], 0) + 1
    for name, count in sorted(by_file.items(), key=lambda item: -item[1]):
        print(f"  {name:20s} {count}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", help="directory to write one YAML file per key")
    parser.add_argument("--consolidated", help="write the complete sorted catalog")
    parser.add_argument("--report", action="store_true")
    parser.add_argument("--show", help="print one key's record")
    arguments = parser.parse_args()

    try:
        import ini_inventory

        adjudications = _engine.load_adjudications()
        adjudication_errors = schema_validation.errors_for(
            adjudications, "adjudications.schema.json", "manual/data/adjudications.yaml")
        if adjudication_errors:
            raise ValueError("\n".join(adjudication_errors))
        keys, records, missing, inventory = _extract_all(
            ini_inventory.load_manifest())
    except (OSError, ValueError) as error:
        print("ACTION REQUIRED", file=sys.stderr)
        for line in str(error).splitlines():
            print(f"  - {line}", file=sys.stderr)
        return 1

    if arguments.show:
        entry = keys.get(arguments.show, {"error": "not found"})
        try:
            print(_engine._yaml().safe_dump(
                entry, sort_keys=False, allow_unicode=True, width=100))
        except ImportError:
            print(json.dumps(entry, indent=2, ensure_ascii=False))
        return 0

    if arguments.consolidated:
        payload = _engine._yaml().safe_dump(
            {key: keys[key] for key in sorted(keys)},
            sort_keys=False, allow_unicode=True, width=100)
        atomic_write_text(arguments.consolidated, payload)

    if arguments.out:
        directory = Path(arguments.out)
        directory.mkdir(parents=True, exist_ok=True)
        for name, entry in keys.items():
            safe_name = re.sub(r"[^\w.-]", "_", name)
            payload = _engine._yaml().safe_dump(
                entry, sort_keys=False, allow_unicode=True, width=100)
            atomic_write_text(directory / f"{safe_name}.yaml", payload)

    if arguments.report or not (arguments.out or arguments.consolidated):
        _report(keys, records, missing, inventory)
    return 0


if __name__ == "__main__":
    sys.exit(main())
