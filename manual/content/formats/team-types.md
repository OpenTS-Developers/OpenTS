---
format_id: teamtypes
title: Team types
summary: Registers team definitions that link an owner, TaskForce, Script, waypoint, and behavior flags.
kind: registry
route: /mapping/team-types/
files:
  - AI.INI
  - AIFS.INI
  - map file
registrations:
  - { section: TeamTypes, id_from: value, entry_section: "<TeamType ID>" }
key_scopes:
  - applies_to: TeamType
source_files:
  - code/teamtype.cpp
  - code/init.cpp
  - code/scenario.cpp
---

`[TeamTypes]` values name the TeamType sections to load, and the number to the left of each line is only that line's name. The keys in each section select its owner, TaskForce, Script, origin waypoint, recruitment rules, and behavior flags.

OpenTS loads `AI.INI`, then `AIFS.INI` when Firestorm is enabled, then the map-local definitions.

```ini title="AI.INI, AIFS.INI, or map file"
[TeamTypes]
0=MyAttackTeam ; example TeamType

[MyAttackTeam]
House=GDI
TaskForce=MyAttackForce ; defined under [TaskForces]
Script=MyAttackScript   ; defined under [ScriptTypes]
Waypoint=A
```
