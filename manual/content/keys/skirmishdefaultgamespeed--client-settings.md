---
key: SkirmishDefaultGameSpeed
scope: client-settings
label: Skirmish default game speed
summary: Starting game speed for a fresh skirmish when the defaults override is on.
see_also: [OverrideSkirmishDefaults, GameSpeed]
when_omitted:
  kind: value
  value: "3"
---

`SkirmishDefaultGameSpeed` is the starting game speed for a skirmish with nothing saved to restore, when `OverrideSkirmishDefaults=yes`. The value is on the same scale as [`GameSpeed`](/keys/gamespeed/).

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
