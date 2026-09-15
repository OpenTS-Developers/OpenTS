---
key: ShareSource
summary: The object type a voxel animation borrows its voxel model from.
see_also: ["ShareBodyData", "ShareTurretData", "ShareBarrelData", "VoxelIndex"]
when_omitted:
  kind: value
  value: none
---

Read only where one of [`ShareBodyData`](/keys/sharebodydata/), [`ShareTurretData`](/keys/shareturretdata/) or [`ShareBarrelData`](/keys/sharebarreldata/) is set. With none of them set the type loads a `.VXL` of its own, named after its [`Image`](/keys/image/#scope-aircrafttype), and this setting is not read at all.

The value is an ObjectType ID, matched against the InfantryType, VehicleType, AircraftType and BuildingType lists in that order. The model is taken by reference rather than copied: the animation points at the very data the named type draws from, so the two always show the same artwork and the borrowing costs no memory of its own.

```ini title="rules.ini"
[VoxelAnims]
20=MYTURRETDEBRIS ; a VoxelAnimType, registered here so the rules create it

[MYTURRETDEBRIS]
ShareTurretData=yes
ShareSource=MYTANK ; a VehicleType registered in [VehicleTypes]
VoxelIndex=0
MinAngularVelocity=10.0
MaxAngularVelocity=14.0
```

A name matching nothing in those four lists leaves the animation holding no model, and so does a named type that has no model for the part asked for. A type drawn from shapes rather than voxels has no model, and a vehicle without a turret has neither turret nor barrel model. Such a piece is still created, still bounces, still damages what it lands on and still expires; it is simply never drawn.

:::danger[Dropping the borrowing in a later file frees the lender's model]
A borrowed model belongs to the type it came from: while the borrowing stands the animation only drops its reference to it and never frees it. A later rules or map file that declares the section again without repeating the sharing flag turns the borrowing off. The section is then read as an owner: the borrowed model is released and one of the animation's own is loaded in its place. The lending object type is then drawn out of freed memory, and the motion data that animates its model is released a second time when the game shuts down, corrupting the heap.
:::
