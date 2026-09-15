---
key: Warhead
scope: voxelanimtype
label: Voxel debris warhead
see_also: ["Damage", "DamageRadius", "ExpireAnim"]
when_omitted:
  kind: value
  value: none
---

The warhead sets [`Damage`](/keys/damage/#scope-voxelanimtype) at both points a voxel animation deals it. One is the bounce: each object in the bounce cell standing within [`DamageRadius`](/keys/damageradius/#scope-voxelanimtype) of it takes the damage. The other is the blast where the piece's life ends on land or above a bridge, which fires only when the type also sets [`ExpireAnim`](/keys/expireanim/#scope-voxelanimtype). Its armor multipliers and its [`Spread`](/keys/spread/#scope-warheadtype) decide what that damage is worth against a given target, and its [`Bright`](/keys/bright/#scope-warheadtype) flag decides whether the blast throws a flash of light.

A piece with no warhead named deals nothing at either point: the blast is refused before it starts, and a bounce skips its damage pass. The piece still bounces and plays its effects, and a meteor still craters or seeds the ground it lands on. Its stored [`Damage`](/keys/damage/#scope-voxelanimtype) is left unchanged.
