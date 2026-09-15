---
key: Spawns
scope: voxelanimtype
label: Voxel meteor spawn type
see_also: ["SpawnCount", "IsMeteor", "IsTiberium"]
when_omitted:
  kind: value
  value: none
---

The voxel animation type a meteor's impact breaks into. The children are all created at once, at the point of impact, and with no owning house. A child with [`IsTiberium=yes`](/keys/istiberium/#scope-voxelanimtype) is therefore drawn in Tiberium colors, where a piece thrown off a destroyed vehicle is not. [`SpawnCount`](/keys/spawncount/#scope-voxelanimtype) fixes how many appear.

The branch belongs to [`IsMeteor=yes`](/keys/ismeteor/#scope-voxelanimtype) alone, and is skipped where the meteor lands in water below the height of a bridge deck, 416 leptons or about a cell and a half above the ground.

A value naming no registered voxel animation is not refused: a type of that name is created on the spot, holding no artwork and every setting at its built-in value, and the impact spawns pieces of it.

:::caution[A type that spawns itself never runs out]
A meteor naming its own type here, or any cycle of meteor types that leads back to it, replaces itself at every impact. At a count of `1` each impact replaces the piece on average; at `2` or more the population grows with every generation, and each piece deforms the ground it lands on in turn.
:::
