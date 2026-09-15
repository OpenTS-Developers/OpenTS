---
key: BounceAnim
scope: voxelanimtype
label: Voxel debris bounce effect
see_also: ["BounceSound", "Elasticity", "DamageRadius"]
when_omitted:
  kind: value
  value: none
---

An animation of the named type is created at the debris' position on every frame the bounce physics report a strike. The strike may be against the ground, a bridge deck, or a building or wall standing in the way. A strike inside a water cell is not one of them: the piece there has its lifetime cleared instead and plays nothing.

The landing that ends the flight does not count either. A contact that leaves the piece with too little motion to keep tracking is reported as settling rather than as a strike, so it takes the expiry path directly. A piece with [`Elasticity=0`](/keys/elasticity/#scope-voxelanimtype) keeps no speed across a contact, so its arrival on the ground settles it and this animation goes unplayed.
