---
key: Height
scope: random-map-generation
label: Generated map height
see_also: ["Width"]
when_omitted:
  kind: value
  value: "0"
  note: The smallest playable area the chosen player count allows.
---

The figure is a size index rather than a cell count. It is divided by three and used to interpolate the generated playable area's depth between a minimum and a maximum chosen by the player count. `0` gives the smallest map for that many players and `3` the largest. At two players that range is 50 to 100 cells; it climbs with each further player, reaching 135 to 175 at eight. The generated map itself is twelve rows deeper than the playable area. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
NumPlayers=4
Height=3
```

When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `0` becomes `0` and one above `3` becomes `3`.
