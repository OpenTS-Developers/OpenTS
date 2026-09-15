---
key: DestroyableCliffs
summary: Tile set holding the two cliff faces that weapon fire can bring down.
see_also: [CollapseChance, SlopeSetPieces, CliffSet]
when_omitted:
  kind: value
  value: "-2"
  note: The role stays unresolved and the theater has no collapsible cliff, since the two tile indices it would name are both negative and no cell can hold them.
---

Exactly two tiles are collapsible: the one at this role and the one after it. Every other cliff face in the theater is permanent, so the set this key names is the whole of what a weapon can bring down. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

A cell with one of those two tiles is tested after damage reaches it from any of:

- an ordinary explosion,
- a sonic wave sweeping the cell,
- a railgun beam that stops on it.

Each test rolls the [`CollapseChance`](/keys/collapsechance/) percentage. On a success the cliff tile is lifted off the cells it covered, two pieces from [`SlopeSetPieces`](/keys/slopesetpieces/) are laid in its place, and movement zones are rebuilt so units can find the new way up. Everything still targeting those cells is retargeted, and rubble is scattered across the wreckage from three hard-coded debris animations, `XGRYMED1`, `XGRYMED2` and `XGRYSML1`.

The two tiles also change how a cell is treated before it collapses. A player may order a selected unit to attack one directly, unless that unit's primary weapon uses a fire warhead. A unit given that order retargets to the closest passable cell so it can approach the rock face. An attacker holding a webbing weapon picks its other weapon against one of these cells.

:::note[The sentinel is `-2`, not `-1`]
Every other tile role in this section falls back to `-1`; this one falls back to `-2`, so the second index it derives lands on `-1` rather than on the theater's first tile. Writing `DestroyableCliffs=-1` does not undo that: a value matching no tile set leaves the role at `-2`, so it behaves exactly as omitting the key does.
:::
