---
format_id: scripts
title: Scripts
summary: Registers ordered team-mission lists containing up to fifty mission and argument pairs.
kind: record
route: /mapping/scripts/
files:
  - AI.INI
  - AIFS.INI
  - map file
section: ScriptTypes
syntax: "<slot>=<mission>,<argument>"
registration: { section: ScriptTypes, id_from: value, entry_section: "<Script ID>" }
fields:
  - { position: 1, label: Mission, value: Team mission index, required: true }
  - { position: 2, label: Argument, value: Integer interpreted by the selected mission, required: true }
key_scopes:
  - applies_to: ScriptType
related:
  - { type: mission, id: TMISSION_ATTACK }
source_files:
  - code/script.cpp
  - code/tmission.cpp
  - code/init.cpp
  - code/scenario.cpp
---

`[ScriptTypes]` values name the Script sections to load. The number to the left of a `[ScriptTypes]` line is only that line's name. Inside a Script section the number is the mission's slot, and each section accepts lines `0` through `49`.

OpenTS loads `AI.INI`, then `AIFS.INI` when Firestorm is enabled, then the map-local definitions. When both files use the same `[ScriptTypes]` number, only the later file's script is registered.

```ini title="AI.INI, AIFS.INI, or map file"
[ScriptTypes]
0=MyAttackScript

[MyAttackScript]
Name=Attack nearest target
0=0,0
```

Lines load in slot order and pack together, so gaps in the numbered slots collapse.
