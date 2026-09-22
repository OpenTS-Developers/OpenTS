---
key: RegionSize
summary: How large a piece of ground a generated map may hold at one height before it is broken up, as a figure from 0 to 100.
see_also: [Accessibility, Ruggedness, Biome]
when_omitted:
  kind: value
  value: "0"
  note: The limit is a tenth of the playable area's width times its height, about a twenty-fifth of the map.
---

The generator carves the map into regions and then repeatedly splits any dry region that is too big. One half of each split is put at a new height. The cliff faces of a generated map are what those height changes leave behind, so the size limit is what decides how much cliff there is. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
RegionSize=20
```

A region is split while it holds more cells than the limit. The limit is a tenth of the playable area's width times its height, plus a hundredth of that product for every point. On a map whose playable area is 100 by 100 cells, the limit at `0` is 1,000 cells and every point adds another 100, so `RegionSize=40` sets a limit of 5,000. A generated map is a diamond and holds about twice its full width-times-height in cells. The split pass ranges over the whole diamond, border included. The limit therefore works out at about a twenty-fifth of the map at `0` and between two fifths and half of it at `100`. A region small enough to keep is marked so that it is never examined again, and a region holding water is never split at all.

When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `0` becomes `0` and one above `100` becomes `100`.

:::caution[The setting does nothing at all on the tundra biome]
The splitting pass is skipped outright when [`Biome`](/keys/biome/) is `0`, the tundra, so on a tundra map the figure changes nothing whatever it is set to. On every other biome the figure stays live across its whole range. Even at `100` the limit reaches only about half the map, so a single large region is still broken up.
:::
