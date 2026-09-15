---
key: ShadowTiles
summary: Second condition a shadow-casting tile set must satisfy before any of its tiles is marked as a caster.
see_also: [ShadowCaster]
when_omitted:
  kind: value
  value: "0"
  note: No tile of the set is marked as a caster, though the set still takes one of the five caster slots.
---

The key is only looked for on a set that already sets [`ShadowCaster=yes`](/keys/shadowcaster/); on any other set it is not read at all. What the figure then does is decided by whether it is zero, and nothing else: any non-zero figure marks every tile of the set as a caster, and zero marks none of them.

```ini title="TEMPERAT.INI"
[TileSet0010]      ; example cliff set
SetName=Cliffs
FileName=CLIFF
TilesInSet=40
ShadowCaster=yes
ShadowTiles=40     ; any non-zero figure has the same effect
```

The name suggests a count of the tiles at the front of the set that cast a shadow. There is no such count. Which tiles cast what is fixed by the shadow table [`ShadowCaster`](/keys/shadowcaster/) describes, and the figure here never reaches it.
