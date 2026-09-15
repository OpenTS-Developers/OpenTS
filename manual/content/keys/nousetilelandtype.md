---
key: NoUseTileLandType
summary: Overlay whose own land type stands instead of the land type of the tile beneath it.
see_also: [Land, Wall, Tiberium]
when_omitted:
  kind: value
  value: "yes"
---

A cell recalculating its attributes starts from the overlay's [`Land`](/keys/land/) and then decides whether to keep it. With the flag set it keeps it and stops there; with the flag off it goes on to the tile and overwrites the land type with the tile's own. `Wall` and `Railroad` are settled before the flag is reached, so an overlay declaring either of those keeps it whatever this key says.

One rule still reaches the cell on the way out. [`CliffBackImpassability`](/keys/cliffbackimpassability/) runs before the early return and rewrites the land type to `Rock` when it is set to `2` and a neighbor stands at least four height levels above the cell. It is `0` unless something sets it, so the exception is opt-in.

```ini title="rules.ini"
[MYRUBBLE]              ; example overlay that should not change the ground
Land=Clear
NoUseTileLandType=no    ; the tile underneath keeps deciding
```

Stopping early does more than fix the land type. The whole tile-driven half of the recalculation is skipped for as long as the overlay is on the cell. The cell does not re-blend against its neighbors, does not attach the animation its tile set names, and does not mark the cells its tile overshadows. Only the cell's ramp is still taken from the tile. Lighting is not part of that half at all: a cell's light comes from light sources and the scenario's ambient level through a separate recalculation, so an overlay with this flag never darkens or extinguishes a lit tile.

Turning the flag off does not always hand the decision to the tile. [`Land`](/keys/land/) covers the case of a `Clear` overlay standing on a cell where Tiberium is growing, which reports `Tiberium` rather than either.
