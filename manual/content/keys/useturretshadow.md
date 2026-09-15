---
key: UseTurretShadow
summary: Parsed shadow flag that the engine never uses.
no_effect: true
see_also: ["Turret", "ShadowIndex", "Voxel"]
when_omitted:
  kind: value
  value: "no"
---

No drawing path reads the flag. A [`Turret=yes`](/keys/turret/) vehicle drawn from shape artwork casts its shadow from the frame at its own body frame plus half the frame count of its shape file: body frame `45` in a `240`-frame file shadows from frame `165`. The shadow follows the hull rather than the gun, whatever `UseTurretShadow` is set to. A voxel vehicle has [`ShadowIndex`](/keys/shadowindex/) to name the section its shadow renders from; a shape turret's shadow frame is computed, and no setting names it.
