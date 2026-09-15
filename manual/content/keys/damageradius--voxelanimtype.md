---
key: DamageRadius
scope: voxelanimtype
label: Voxel debris bounce reach
see_also: ["Damage", "Warhead", "BounceAnim", "MinDamage"]
when_omitted:
  kind: value
  value: "0"
---

A whole number of leptons, 256 to a cell. On every frame the debris strikes the ground outside water, each object occupying the cell it struck is measured against this reach, and one that falls inside it takes [`Damage`](/keys/damage/#scope-voxelanimtype) through the piece's [`Warhead`](/keys/warhead/#scope-voxelanimtype). The piece deals this damage only if its type names a warhead.

```ini title="rules.ini"
[MetalShard]     ; a voxel animation type
Warhead=AP
Damage=15
DamageRadius=256 ; objects within a cell of the strike take the damage
```

The distance compared is the sum of the two horizontal separations rather than the straight line between them, so the reach is a diamond and not a circle. `0` therefore reaches only an object centered exactly on the point of impact. Height is not part of the comparison at all. Only occupants of the one cell struck are examined, so a reach wider than a cell still cannot touch anything standing next door.

A hit is not dealt flat. The damage passes through the piece's warhead like any other damage: the target's armor type modifies it, and it falls off with the object's distance from the impact point, on the same falloff the blast applies.

The blast at the end of the piece's life is a separate matter: it reaches every object in the impact cell and the eight around it, with the warhead's own [`Spread`](/keys/spread/#scope-warheadtype) deciding how the damage falls off across them. It does not read this setting at all.
