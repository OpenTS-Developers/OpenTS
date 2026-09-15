---
key: WaterfallWest
summary: The tile set that supplies the four pieces of a waterfall running west.
see_also: [WaterfallEast, WaterfallNorth, WaterfallSouth]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The role covers four consecutive tiles, and two tests read them. A water test counts every one of the four as holding water, so a vehicle standing on any of them refuses to take a passenger aboard. A cliff test counts them as rock face, sparing only the subtiles at the ends of the fall that spill out onto ordinary ground. On this set those are subtiles `1` and `3` of its first and last piece. The pieces are laid by the [random map generator](/formats/map-seed/), which puts the first and last at the two ends of a west-running fall and fills the drop between them with the middle two.

[`WaterfallEast`](/keys/waterfalleast/) covers both tests as all four roles share them, including the order the roles are taken in, what an unresolved role accepts, and why the moving water is not this key's doing.
