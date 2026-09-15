---
key: ShareBodyData
summary: Borrows the voxel model of another object type's body instead of loading one of the animation's own.
see_also: ["ShareSource", "ShareTurretData", "ShareBarrelData", "VoxelIndex"]
when_omitted:
  kind: value
  value: "no"
---

The model comes from the object type named by [`ShareSource`](/keys/sharesource/) rather than from a `.VXL` file of the animation's own. This flag claims the source's body model, the one the source's own instances are drawn from.

The three sharing flags are tested in a fixed order and the first one set decides the part: body, then turret, then barrel. Setting more than one is therefore not additive, and a type with both this flag and [`ShareTurretData=yes`](/keys/shareturretdata/) takes the body.

:::danger[Dropping the flag in a later file frees the lender's model]
A later rules or map file that declares the same section again without repeating this flag turns the borrowing off. The section is then read as owning its model: the borrowed one is released, and a fresh one is loaded in its place. The lending object type is then drawn out of freed memory, and the motion data that animates its model is released a second time when the game shuts down, corrupting the heap. [`ShareSource`](/keys/sharesource/) covers the borrowing in full.
:::
