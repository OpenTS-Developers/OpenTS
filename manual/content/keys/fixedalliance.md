---
key: FixedAlliance
summary: Parsed flag that locks nothing.
no_effect: true
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

`FixedAlliance` has no effect in any game type. A locked alliance would disable the [`ToggleAlliance`](/commands/togglealliance/) command, which forms or breaks an alliance with the selected object's owner. That command works only outside single-player missions, and only a single-player mission reads this entry.

No game setup in OpenTS locks alliances either. Whether players can ally is decided by the game's alliance option.
