---
key: StartTurretFrame
summary: The frame a shape-drawn vehicle's turret artwork begins at.
see_also: ["TurretFacings", "WalkFrames", "Facings", "Turret"]
when_omitted:
  kind: computed
  note: 8 × WalkFrames, counted from frame 0, whatever Facings holds.
---

The turret strip runs from here, one frame for each of the vehicle's [`TurretFacings`](/keys/turretfacings/).

The derived default leaves room for eight walk blocks, and that eight is fixed. A vehicle cut into more than eight [`Facings`](/keys/facings/) therefore has walk blocks that would run through its own turret strip, so its artwork either puts the walk block after the strip or names this key to move the strip elsewhere.

```ini title="art.ini"
[MYTANK] ; the Image ID of a shape-drawn UnitType
Facings=32
WalkFrames=3
                  ; the derived strip: frames 24-55, eight blocks of 3
StartWalkFrame=56 ; the 32 walk blocks begin after it
```
