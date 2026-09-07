---
key: TurretFacings
summary: The number of facings a shape-drawn vehicle's turret artwork is cut into.
see_also: ["StartTurretFrame", "Facings", "Turret", "WalkFrames"]
when_omitted:
  kind: value
  value: "32"
---

Only a [`Turret=yes`](/keys/turret/) vehicle drawn from shape artwork reads this figure, and it owes nothing to the hull's [`Facings`](/keys/facings/). Every stock vehicle cuts its body into eight facings and its turret into thirty-two.

Per-facing drawing happens at `8`, `16`, `32` and `64`. The facing drawn is the turret's own heading rounded to that many compass points and then advanced by an eighth of a turn, so a turret pointing northwest draws facing 0 at every count. At any other value the facing is fixed at 0 and the turret draws the first frame of its strip however it is aimed.

```ini title="art.ini"
[MYTANK] ; the Image ID of a shape-drawn UnitType
Facings=8        ; eight body facings
WalkFrames=15
TurretFacings=32 ; thirty-two turret frames, from frame 120
```

[`StartTurretFrame`](/keys/startturretframe/) names the frame that strip begins at.
