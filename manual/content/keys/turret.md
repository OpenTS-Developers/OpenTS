---
key: Turret
summary: Gives a vehicle or aircraft a turret that aims and fires independently of its body.
see_also: ["TurretSpins", "ROT", "RotCount"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle or aircraft keeps a second facing alongside its body facing, and the flag decides what that second facing is for. With a turret, the second facing tracks the target, the shot leaves along it, and the turret artwork is drawn at it. It turns at the same [`ROT`](/keys/rot/#scope-aircrafttype) as the hull, and the hull is free to drive one way while the gun points another. Without a turret, a UnitType is marked as having its turret locked. It then has to bring the hull itself round onto the target before it may fire, and it can do that only while it is stopped and holds no move order.

A UnitType that is not drawn from a voxel model loads a companion turret model named for its image with `TUR` appended when the flag is set. That is how a shape-drawn type gets a voxel gun.

On a BuildingType the flag does not create a second facing (a structure aims its turret with its only facing), but three other things read it. A structure counts as turret-equipped if any upgrade plug fitted into it sets the flag, even when its own section does not, so a plug can bring a gun to a structure that has none. Removing an upgrade plug ends the turret animation and clears the structure's turret index. And the flag sets the structure's stored rotation frame count to 32 rather than 1, overwriting whatever [`RotCount`](/keys/rotcount/) supplied; nothing reads that number.
