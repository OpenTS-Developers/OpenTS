---
key: IsTiberium
scope: voxelanimtype
label: Voxel Tiberium debris
see_also: ["system:tiberium", "IsMeteor", "Spawns"]
when_omitted:
  kind: value
  value: "no"
---

Two unrelated effects come from the one flag.

Tiberium is seeded where the piece comes down on solid ground: the ring of eight cells around a meteor's impact, or the single cell beneath any other piece. [Other sources of Tiberium](/systems/tiberium/#other-sources-of-tiberium) covers which type is planted and at what growth stage. A piece that comes down in water, or onto a bridge deck, seeds nothing.

The piece is also drawn through the Tiberium color table instead of the plain voxel one, and takes its lighting from the cell beneath it rather than the full brightness an unowned piece otherwise gets. The recoloring reaches only a piece with no owning house, because a house color scheme is chosen ahead of it. Debris thrown off a destroyed vehicle takes that vehicle's house and keeps its colors. Debris a meteor spawns, an exploding barrel throws, or a trigger action places has no house and is recolored.
