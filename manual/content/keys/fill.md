---
key: Fill
summary: The tile every cell of the map starts out as, either clear ground or water.
see_also: [Size, Level, Theater]
when_omitted:
  kind: value
  value: "Clear"
---

The value is compared against `Water` alone, without regard to case. That one value fills the map with the water tile set. Every other value, including a misspelling of it and the `Clear` that the read falls back on, fills the map with clear ground. The cells are filled as the map's dimensions are established, before any of the terrain, overlay or object entries are read. Anything those entries place is therefore laid over the fill rather than into it. The fill resolves its tile before [`Theater`](/keys/theater/) is read and the theater's own tile set is loaded.

```ini title="map file"
[Map]
Size=0,0,64,64
Fill=Water
```

Every filled cell takes the first tile of the chosen set. A water map filled this way has no shoreline of its own: the entries that follow have to lay one down.
