---
key: Inert
summary: Stops ordinary weapon hits in the scenario from doing damage.
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

`Inert=yes` stops weapon fire from doing damage. Ordinary hits deal no damage, and an explosion damages nothing around it, walls and bridge spans included. Healing weapons stop healing as well. Weapons still reload and fire, projectiles still travel, and firing and impact effects still play. A [wide-area blast](/systems/warheads/#the-wide-area-blast) can still leave a crater.

Damage the game forces through is not affected. A hunter-seeker that detonates still damages its target, and infantry standing in Tiberium still take damage from it and can die.

:::caution[The entry is read in campaigns only]
Only a single-player mission reads `[SpecialFlags]` from the map. In every other game type, weapons do damage whatever the map says, unless a Debug build was started with the [`-XI`](/using/command-line/inert/) launch option.
:::
