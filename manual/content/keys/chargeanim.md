---
key: ChargeAnim
summary: Drives a turretless structure's turret animation from its charge counter.
see_also: ["TurretAnim", "TurretChargeAnimRate", "TurretAnimIsExclusive", "Charges"]
when_omitted:
  kind: value
  value: "no"
---

The flag does not make a structure charge. The weapon decides that: a [`Charges=yes`](/keys/charges/) primary weapon is what puts a structure through a charge-up before it may fire, and [`TurretChargeAnimRate`](/keys/turretchargeanimrate/) is how fast that charge runs.

What the flag does is let a structure with no turret take the paths written for one. A `ChargeAnim=yes` type has the animation named by [`TurretAnim=`](/keys/turretanim/) created for it. That animation's frame is reset on every pass from a charge counter the structure keeps apart from its artwork sequence, stepped at `TurretChargeAnimRate`. The frame follows the charge rather than a facing, so the animation advances with the charge instead of pointing at a target.

The animation is created as the structure opens, unless the type sets [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) or [`TurretAnimIsExclusive=yes`](/keys/turretanimisexclusive/). On either of those nothing is created at that moment and the animation appears only once the structure begins charging.

```ini title="art.ini"
[MYOBEL] ; example obelisk, drawn from its own Image ID
ChargeAnim=yes
```

```ini title="rules.ini"
[MYOBEL]
TurretAnim=MYOBEL_A ; the animation the charge counter drives
Primary=MyLaser     ; a Charges=yes WeaponType
```

Outside construction and deconstruction the structure's own artwork sequence, the idle sequence included, is no longer restarted when it runs past its last frame, and the wrap no longer raises the signal that a queued mission may start. A structure with a weapon raises that signal on every pass it spends guarding regardless, so a queued mission is not held up by its absence.
