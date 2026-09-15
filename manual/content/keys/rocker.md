---
key: Rocker
summary: The blast tips the vehicles around it, with the force taken from the damage.
see_also: [Weight]
when_omitted:
  kind: value
  value: "no"
---

The force is a hundredth of the blast's damage, held at four. Damage of 30 or less produces no force worth applying and nothing tips at all; damage of 400 and above all tips by the same amount.

```ini title="rules.ini"
[MyShellWH] ; example WarheadType
Rocker=yes
```

Every object standing in the seven-by-seven block of cells centered on the blast is offered the disturbance, but only one drawn as a voxel takes it; infantry and buildings drawn from shape art are left alone. The tilt is the force divided by the type's [`Weight`](/keys/weight/), reduced by the object's distance from the blast and dropped altogether once that leaves it negligible. A heavy vehicle tips less than a light one at the same range. That figure is capped as well, so a light vehicle close to a strong blast tips no faster than the cap allows. An object standing in the blast's own cell is tipped away from whatever fired the shot rather than away from the impact point.

The tilt is held at 45 degrees, forward and sideways alike, and eases back to level over the frames that follow; a vehicle that is crushing something at the time is held to 18 degrees forward instead. The disturbance costs no strength of its own.
