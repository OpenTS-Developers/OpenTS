---
key: Foundation
scope: terraintype
label: Terrain footprint
when_omitted:
  kind: value
  value: "1x1"
---

The value is one of the same 22 size names a structure uses, matched without regard to case, and anything else resolves to `1x1`. It picks the list of cells the terrain object stands on, and the engine marks those cells occupied. A vehicle's entry check refuses any cell the terrain object claims, unless the vehicle has a weapon that destroys wood; [Movement and terrain](/systems/movement-and-terrain/#why-a-cell-refuses-a-vehicle) owns that test.

Terrain objects have their own table of footprints, and it is filled in only as far as the eighth name.

:::danger[A terrain object may not use a footprint past the eighth name]
Only `1x1`, `2x1`, `1x2`, `2x2`, `2x3`, `3x2`, `3x3` and `3x5` have cell lists. Every larger name selects a row the table never fills in. Those rows hold nothing but zeros and no end marker.

Placing such an object claims its top-left cell thirty-one times and nothing more. The placement routine copies the list into a capped buffer first, and the copy's last entry stands in for the missing end marker. The scans that walk the raw list have no bound. They read that same zero entry as many as a hundred and forty times, then continue past the end of the table. Past that point they test cells at offsets taken from whatever data the program stores there. A map lookup never faults, so the outcome is whatever follows the table rather than a clean refusal or a bounds error.
:::

:::caution[On a terrain object 3x5 is a four-by-two block]
The eighth row holds four cells by two rather than the three by five its name promises, so a TerrainType given `Foundation=3x5` stands on a block of that shape instead.
:::
