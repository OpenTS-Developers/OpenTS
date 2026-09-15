---
key: Voxel
summary: Draws the type from a voxel model rather than from a shape file.
see_also: ["Image", "ShadowIndex", "Turret", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle, aircraft, infantry, building, or bullet type marked here loads `<Image ID>.VXL` together with its `<Image ID>.HVA` motion file.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
Voxel=yes ; draws MYTANK.VXL and MYTANK.HVA
```

A [`Turret=yes`](/keys/turret/) vehicle also loads `<Image ID>TUR` for its turret and `<Image ID>BARL` for its barrel, each as a `.VXL` and `.HVA` pair. A voxel type that is not a vehicle looks for both pairs whether or not it has a turret. A vehicle with no turret looks for neither pair. The exception is the amphibious transport, whose rules section is named exactly `APC`: it looks for an `<Image ID>W` pair instead. A missing companion pair is simply not drawn, so shipping neither companion pair is safe. A half pair is the dangerous case: a companion `.VXL` present while its `.HVA` is missing fails the load and throws the whole voxel set away, main model included, leaving the type with no model at all.

Voxels have no theater variants. [`Theater=yes`](/keys/theater/) and [`NewTheater=yes`](/keys/newtheater/) rename the file for the shared shape fetch, and a voxel type skips that fetch, so neither flag changes a voxel's artwork. A BulletType is the exception: it makes that fetch on its own account unless it is [`Inviso=yes`](/keys/inviso/), so a voxel projectile keeps its shape as well.

:::caution[A model that fails to load leaves nothing behind]
A vehicle marked as a voxel is drawn from its model or not at all. There is no fallback to the shape file, so a misspelled Image ID or a missing `.VXL` leaves the vehicle invisible while it goes on fighting.
:::
