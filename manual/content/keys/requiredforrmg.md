---
key: RequiredForRMG
summary: Tile set whose artwork is kept in memory on a generated map even while no cell uses it.
see_also: [AllowToPlace, TilesInSet]
when_omitted:
  kind: value
  value: "no"
---

Tile artwork is trimmed after the map has been read. The loader counts how many cells use each tile type and throws away the image of every type the count leaves at zero, so only the part of a theater a scenario actually needs stays resident. On a generated map that count is misleading, because the generator goes on placing tiles after the trim. A set with this flag is exempt from the trim on a generated map, and its artwork is loaded instead of discarded.

```ini title="TEMPERAT.INI"
[TileSet0042]        ; example set the generator lays down as it works
SetName=Shore pieces
FileName=SHORE
TilesInSet=12
RequiredForRMG=yes
```

The exemption applies only on a generated map. On an ordinary scenario the trim runs as though the flag were not there, and a set nothing uses loses its artwork whatever it says. Reaching a draw reloads it, so the flag governs when the read happens rather than whether the artwork can be had at all.
