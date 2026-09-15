---
key: NonMarbleMadness
summary: Tile-set number whose only live consequence is whether the alternate artwork extension is tried when a tile file is missing.
see_also: [MarbleMadness, FileName]
when_omitted:
  kind: value
  value: "65535"
  note: A non-zero figure, so the alternate extension is tried.
---

Two different things happen to the value, and only one of them matters.

The figure is tested for being non-zero while the set's tiles are looked for on disk. When a tile's theater file is missing and the figure is non-zero, the same name is tried again with the alternate terrain extension (`.MMT` in the temperate theater, `.MMS` in snow). The tile takes that artwork if it is there. Setting the key to `0` suppresses that second attempt, and the tile is left with no artwork of its own. Nothing about the *number* is used here; only whether it is zero.

```ini title="TEMPERAT.INI"
[TileSet0631]         ; example set that has no alternate artwork
SetName=Riverbank cliffs
FileName=RVCLIF
TilesInSet=8
NonMarbleMadness=0    ; do not look for RVCLIF01.MMT and its fellows
```

The figure is also treated as a tile-set number and resolved, once the file has been read, into a tile index in the named set. That result is never read by anything, and [`MarbleMadness`](/keys/marblemadness/), which is resolved the same way, describes what the resolution pass does with a number the theater has no set for.
