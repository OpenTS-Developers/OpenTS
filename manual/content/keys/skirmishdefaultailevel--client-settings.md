---
key: SkirmishDefaultAILevel
scope: client-settings
label: Skirmish default AI level
summary: Starting AI difficulty for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "0"
---

`SkirmishDefaultAILevel` is the starting AI difficulty for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. The level runs from 0 for the easiest to 2 for the hardest.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
