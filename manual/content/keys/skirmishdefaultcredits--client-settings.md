---
key: SkirmishDefaultCredits
scope: client-settings
label: Skirmish default credits
summary: Starting credits for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "5000"
---

`SkirmishDefaultCredits` is the starting credits for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
