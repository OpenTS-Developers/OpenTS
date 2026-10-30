---
key: SkirmishDefaultAIPlayers
scope: client-settings
label: Skirmish default AI players
summary: Starting number of AI opponents for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "1"
---

`SkirmishDefaultAIPlayers` is the starting number of AI opponents for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. The number runs from 1 to 7.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
