---
key: SkirmishDefaultTechLevel
scope: client-settings
label: Skirmish default tech level
summary: Starting tech level for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "10"
---

`SkirmishDefaultTechLevel` is the starting tech level for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. The level runs from 1 to 10.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
