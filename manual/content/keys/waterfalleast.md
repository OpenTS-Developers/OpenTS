---
key: WaterfallEast
summary: The tile set that supplies the four pieces of a waterfall running east.
see_also: [WaterfallWest, WaterfallNorth, WaterfallSouth, WaterSet]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

A water test and a cliff test read the four waterfall roles, and both take them in the order east, west, south, north. The cliff test exists in two implementations, only one of which checks that the role resolved.

The first is the water test, which any of the four pieces satisfies. Outside the random map generator only one thing asks it: a vehicle offered a passenger refuses the load while it stands on a tile holding water or on a shore piece. A transport on a waterfall tile therefore answers that it cannot take anyone aboard. The second is the cliff test, which counts every piece of a waterfall as rock face apart from the two subtiles at each end of the fall that spill out onto ordinary ground. On this set those are subtiles `0` and `4` of its first and last piece. That test is reached only from the map generator's terrain passes, which is also where the pieces are laid. The [random map generator](/formats/map-seed/) puts the first and last piece at the two ends of a fall and fills the drop between them with the middle two.

The moving water is not this key's doing. A tile animates from the `Tile<NN>Anim` entries its own set has in [the theater control file](/formats/theater-control/), which any tile set may have. A fall keeps running whether or not a role points at it.

:::caution[An unresolved set claims the theater's first tiles]
Nothing guards the water test against a role that no tile set answered. Left unresolved, any one of the four accepts every tile index from `0` through `2`, so the first three tiles the theater loads count as holding water — and a transport parked on one of them will not load.
:::

:::tip[Resolving the roles without waterfall art]
Any real binding is safer than none. The role's whole contribution to the water test is the four tile indexes that start at the bound set's first tile. The cliff test reads those same tiles as rock face apart from the spared subtiles; only random map generation asks it. Bound to the open-water tiles `WaterSet` names, those tiles already answer as water, so the water test sees no difference.
:::
