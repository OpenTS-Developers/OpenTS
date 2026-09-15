---
key: Meteorites
summary: Parsed flag that schedules nothing.
no_effect: true
see_also: ["system:tiberium"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

The name promises the meteor showers that seed fresh Tiberium as a game runs. The flag is parsed with the rest of the scenario's special options and never read: a shower falls only where a trigger action calls for one, whether the flag is set or clear.
