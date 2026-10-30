---
key: OverrideSkirmishDefaults
scope: client-settings
label: Override skirmish defaults
summary: Starts a skirmish with nothing saved to restore from the SkirmishDefault* values.
see_also: [PersistSkirmishSettings]
when_omitted:
  kind: value
  value: "yes"
---

`OverrideSkirmishDefaults=yes` starts a skirmish with nothing saved to restore from the `SkirmishDefault*` values instead of the engine's built-in defaults. A saved setup still wins: when `PersistSkirmishSettings=yes` and a setup was saved, that setup is restored regardless of this key.

`no` starts a fresh skirmish from the engine's built-in defaults.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
