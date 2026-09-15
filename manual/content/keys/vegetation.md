---
key: Vegetation
summary: How thickly a generated map is planted with green ground and trees, as a figure from 0 to 100.
see_also: [Biome, UrbanPresence]
when_omitted:
  kind: value
  value: "0"
  note: The fallback of `0` seeds no green ground and no woods, leaving the ground cover to the fixed rough and sand chances.
---

Each cell is given a chance of each kind of ground cover before any of it is planted, and the figure scales those chances as a percentage. It reaches the green ground and the woods only: the rough ground and the sand are given fixed chances per biome and are laid down at the same rate whatever the figure says. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Biome=2
Vegetation=70
```

The chances are small even at the full figure, and differ per biome. At that figure a temperate or mutated cell has one chance in fifty of green ground and about one in three hundred of a wood. A desert cell has no chance of green ground at all and is given sand instead. A cell on a shoreline is treated far more generously: its green and wood chances are the scaled figures multiplied by ten, which is what puts the trees along the water. The two snow biomes keep to their own figures. Green ground has no chance there either, and at the full figure a wood has one chance in two thousand on tundra and about one in six hundred and fifty on taiga, with no shoreline boost.

At `0` no green ground and no woods are seeded anywhere, and the map is left with only its rough patches, its sand and whatever the settlement and tiberium passes add.
