---
key: SkirmishDefaultCrates
scope: client-settings
label: Skirmish default crates
summary: Starting crates option for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "yes"
---

`SkirmishDefaultCrates=yes` turns the crates option on for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. `no` turns it off.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
