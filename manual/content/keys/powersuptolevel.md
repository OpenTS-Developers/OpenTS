---
key: PowersUpToLevel
summary: How many upgrade slots the plug consumes at once, or -1 for one slot in the next free position.
when_omitted:
  kind: value
  value: "-1"
---

At `-1` the plug takes the next free slot and may be installed on a host that already has plugs, up to the host's [`Upgrades`](/keys/upgrades/) count.

A value of `1`, `2` or `3` instead fills that many slots in one placement, and installation is then accepted only on a host with no plugs at all. The value counts slots to consume; it does not name the slot to occupy. Any other value refuses installation outright.

:::danger[A multi-slot plug leaves the slots beneath it empty]
Only the highest slot filled records the plug, so its [`Power=`](/keys/power/#scope-buildingtype), weapon and superweapon count once however many slots it consumed. Selling or deconstructing the host then walks the slots from the top down and reads each one without checking that it holds a plug, and the first empty slot crashes the game. The danger is confined to `2` and `3`, because a value of `1` fills exactly one slot and leaves nothing empty beneath it. The fill stops at the host's `Upgrades` count, so a single-slot host stays safe. A value of `3` on a two-slot host still leaves an empty slot beneath the plug. Keep multi-slot values off any host that can be sold.
:::
