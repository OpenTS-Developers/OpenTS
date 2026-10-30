---
key: SkirmishDefaultUnitCount
scope: client-settings
label: Skirmish default unit count
summary: Starting unit count for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "1"
---

`SkirmishDefaultUnitCount` is the starting unit count for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. The count runs from 1 to 10.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
