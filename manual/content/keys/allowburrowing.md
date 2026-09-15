---
key: AllowBurrowing
summary: Ground that a subterranean unit may dive into or surface through.
see_also: [Morphable, AllowTiberium]
when_omitted:
  kind: value
  value: "yes"
---

The value is read once per tile set and applied to every tile the set produces. A cell answers that a unit may burrow there under **All of:**

- the tile allows burrowing;
- the cell is unramped;
- the cell is neither under a bridge nor remembered as having been;
- no structure stands on it;
- no terrain object stands on it.

Both ends of an underground move are tested: the cell the unit dives at and the cell it means to come up in.

```ini title="TEMPERAT.INI"
[TileSet0631]        ; example cliff set
SetName=Riverbank cliffs
FileName=RVCLIF
TilesInSet=8
AllowBurrowing=no    ; nothing surfaces out of a cliff face
```

:::caution[The test is skipped outside the playable area]
A cell outside the playable area answers yes before the tile is checked at all, as does a cell whose tile index is unresolved. Only ground inside the playable area is actually protected by the flag.
:::
