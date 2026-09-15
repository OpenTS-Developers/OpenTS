---
key: Width
scope: random-map-generation
label: Generated map width
see_also: ["Height", "NumPlayers"]
when_omitted:
  kind: value
  value: "0"
  note: The fallback of `0` is the smallest size index, which builds the smallest playable area the player count allows.
---

The figure is a size index rather than a cell count. It is turned into one exactly as [`Height`](/keys/height/#scope-random-map-generation) describes, from the same figures: the two share their minimum and maximum tables, so a square pair of indices gives a square playable area. The generated map itself is four columns wider than the playable area. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
NumPlayers=4
Width=3
```

The index is also compared directly, alongside the height and the player count, when the map generator dialog decides whether a fresh preview can reuse the terrain it already has. Changing it therefore costs a full rebuild even where the resulting cell count would have been the same.

On a mutated map the two indices are added to one another as small whole numbers to scale the growths. The mold count is drawn from three up to a quarter of `(Width + 1) × (Height + 1)` plus four. The crystal count is drawn from six up to that product plus eight.

The figure is held to `0` through `3` on the same paths as the height, and left alone on the same one: a seed file read as the game starts. An out-of-range figure reads past no table, unlike [`Time`](/keys/time/): it stretches the interpolation fraction [`Height`](/keys/height/#scope-random-map-generation) describes, and the map scales with it.
