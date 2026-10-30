---
key: SkirmishDefaultRedeploy
scope: client-settings
label: Skirmish default re-deployable MCV
summary: Starting re-deployable-MCV option for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "no"
---

`SkirmishDefaultRedeploy=yes` turns the re-deployable-MCV option on for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. `no` turns it off.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
