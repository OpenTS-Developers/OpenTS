---
key: MiscPaveTile
summary: Fourteen-tile set of pavement patches and weathering used to dress paved ground.
see_also: [PaveTile, ClearToPaveLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Thirteen of the fourteen tiles have a use, split in two groups; offset 7 is placed by nothing. Offsets 0 through 6 are the small patches the random map generator scatters over a generated town. The last three of those are placed only where a two-by-two block of plain pavement is free. Offsets 8 through 13 are the large weathering patches, tiled in two-cell steps across one of five fixed rectangles. They are refused on any cell that is neither plain pavement nor one of offsets 0 through 7.

Unlike pavement proper these tiles have no blend family of their own. They instead count as pavement when [`ClearToPaveLat`](/keys/cleartopavelat/) examines a neighbor, so dressing a paved square with them does not make the square blend against itself.

The terrain test that asks whether a cell holds one of these tiles is not gated on the role resolving: left unresolved, it accepts everything up to index 12, so the theater's first thirteen tiles answer yes.
