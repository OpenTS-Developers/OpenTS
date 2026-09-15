---
key: HoverPad
summary: Hands the structure a free aircraft as it opens for business.
see_also: [PadAircraft, SeparateAircraft, Helipad, AIIonCannonHelipadValue]
when_omitted:
  kind: value
  value: "no"
---

A structure that opens for business having been built rather than captured creates one aircraft of the first [`PadAircraft`](/keys/padaircraft/) entry for its own house. The aircraft appears at the structure's center at ground level, facing the direction its type poses at, is put on guard, and enters radio contact with the structure, which tethers it. [`SeparateAircraft=yes`](/keys/separateaircraft/) suppresses the gift outright.

The flag is also the last of the structure tests behind [the rating a computer house gives each candidate](/systems/superweapons/#the-computers-use) for its ion cannon, where it selects [`AIIonCannonHelipadValue`](/keys/aiioncannonhelipadvalue/).

:::caution[This is not the flag that lets an aircraft dock]
Accepting an aircraft as a docking target, and being approached at a docking cell, are what [`Helipad=yes`](/keys/helipad/) gives a structure. The two are read independently, so a structure with only this one hands out an aircraft that has nowhere of its own to land.
:::

The free aircraft comes from the first entry of [`PadAircraft`](/keys/padaircraft/). An empty list hands out nothing, so the structure opens without its aircraft.
