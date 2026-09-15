---
key: TurretOffset
summary: Moves an object's turret forward from its center along the centerline.
see_also: ["PrimaryFireFLH", "SecondaryFireFLH", "Turret", "Voxel"]
when_omitted:
  kind: value
  value: "0"
---

The distance is in leptons, 256 to a cell. A positive value pushes the turret toward the front; a negative one pulls it back.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
TurretOffset=-40 ; the turret sits behind the middle of the hull
```

Firing adds the distance to the `X` component of every weapon slot's firing offset, so both the mounting point and the muzzle move with the turret and neither [`PrimaryFireFLH`](/keys/primaryfireflh/) nor [`SecondaryFireFLH`](/keys/secondaryfireflh/) has to repeat the displacement. A vehicle's drawn voxel turret and voxel barrel are moved out by this distance along the hull's heading and then turned about that point. The turret pivots where its artwork sits rather than at the middle of the body. A building displaces its turret art along the turret's own heading instead, which matches where it fires from. An aircraft has no turret art; the key moves its firing offsets alone, measured from the direction its weapon faces.

:::caution[A vehicle displaces its shot and its turret art along different headings]
The firing offset is measured in the turret's frame, so the displacement follows the gun. A vehicle's drawn turret is displaced along the hull's heading first and turned about that point afterward. The two agree while the turret is aligned with the hull and separate as it swings away from it.
:::
