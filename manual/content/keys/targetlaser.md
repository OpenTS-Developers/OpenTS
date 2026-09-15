---
key: TargetLaser
summary: Draws a dashed sighting line from a firing vehicle to where its shot is aimed.
see_also: [Primary, "system:action-lines", TargetLaserTime, TargetLaserColor]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYSNIPERTANK] ; a UnitType registered in [VehicleTypes]
TargetLaser=yes
```

Every shot the object fires starts a timer of [`TargetLaserTime`](/keys/targetlasertime/) frames (fifteen, one second, unless `UI.INI` says otherwise). While that timer runs, the object draws a dashed line from its turret to the point its shot is aimed at, with a small marker at each end. The line is a dark red unless `UI.INI` restyles it; [Action lines](/systems/action-lines/) lists the keys. It follows the aim, not the projectile, so it tracks a moving target for as long as the object keeps hold of it. It disappears the moment the object loses its target.

Two conditions narrow it well below what the key's presence on every object type suggests. The timer is started only for a house the local player controls, so an enemy's sighting line is never drawn. Only a vehicle draws the line at all, so setting the key on an aircraft, a structure or an infantryman starts a timer that nothing reads.
