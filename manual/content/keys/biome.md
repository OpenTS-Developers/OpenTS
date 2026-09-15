---
key: Biome
summary: The kind of country a generated map is laid out in, as a position from 0 through 4.
see_also: [Time, Vegetation, WaterAmount, UrbanPresence, Theater]
when_omitted:
  kind: value
  value: "0"
  note: The tundra, so the map is built in the snow theater and the region split pass is skipped outright.
---

The figure picks the kind of country the map is laid out in and settles the theater it is built in. `0` tundra and `1` taiga are built in the snow theater, `2` temperate, `3` desert and `4` mutated in the temperate one. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Biome=2
```

Almost every later pass reads it. A snow theater map has its ambient light cut to three quarters of what the time of day would otherwise give. Tundra alone gets ice and arctic rivers and lakes. Tundra and taiga share the arctic ground cover and the rural settlements, while temperate, desert and mutated get ordinary rivers and lakes, ordinary ground cover and full urban areas. The share of the map that water may claim is chosen per biome, and desert claims the least of the five by a wide margin and grows no river at all. Mutated adds mold and crystal growths, and its buildings are the ones left standing in the worst condition.

Only the map generator dialog and a tournament territory narrow the figure. The dialog offers mutated only where the Firestorm addon is present, and the settings taken off it are held to `0` through `4` with mutated demoted to temperate when that addon is absent. A seed file read as the game starts goes through neither, so a figure written by hand reaches the generator as it stands.

:::danger[A figure outside 0 through 4 reads past several tables]
The figure indexes fixed tables of five entries, without a bounds check, from the very first steps of generation onward: the theater name, the fill tile and the share of the map that may become water. A figure of `5` or more, or a negative one, reads past all of them, and the theater name that comes back is used to load the map's tile set.
:::
