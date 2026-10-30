---
key: PersistSkirmishSettings
scope: client-settings
label: Persist skirmish settings
summary: Reopens the skirmish setup screen with the choices it had when it last closed.
see_also: [OverrideSkirmishDefaults]
when_omitted:
  kind: value
  value: "yes"
---

`PersistSkirmishSettings=yes` reopens the skirmish setup screen with the map and game options it held when it last closed, so a setup survives quitting and relaunching the game. The engine stores the setup in the `[Skirmish]` section of `SUN.INI`. A saved map that has since been removed falls back to the first scenario.

`no` opens the screen with its defaults every time.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
