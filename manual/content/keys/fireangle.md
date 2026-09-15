---
key: FireAngle
summary: The resting elevation of the object's barrel.
when_omitted:
  kind: context-dependent
  note: The barrel rests 11.25 degrees above level. A rules.ini FireAngle on the type stands in first, read as a raw direction-step count rather than degrees.
---

An aircraft, vehicle, or infantryman returns the barrel to this elevation while it has no target. A building does not; when a building has no target its barrel keeps the elevation it was last aimed at.

When a weapon lobs its shot and no ballistic arc reaches the target, the barrel also falls back to this elevation instead of aiming flat.

The value is an angle in degrees. Zero is level, positive raises the barrel, 90 points it straight up, and negative aims it below level. The engine stores the value on its 256-step direction scale, about 1.4 degrees to a step.

```ini title="art.ini"
[MYARTY] ; the Image ID of a UnitType
FireAngle=35 ; the barrel rests 35 degrees above level
```
