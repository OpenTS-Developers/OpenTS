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

When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `0` becomes `0` and one above `3` becomes `3`.
