---
key: Arm
summary: How long after launch a projectile's proximity fuse is held shut, in game frames.
see_also: [ROT, Proximity]
when_omitted:
  kind: value
  value: "0"
---

The fuse is set on the target's position at the moment of launch, not on the predicted aim point, and it does not follow a target that moves afterward. The fuse trips when the projectile comes within 64 leptons of that point, height included, and again when it is within two cells of that point and has started to move away from it. While the arming count is still running the fuse reports nothing at all, so the projectile flies on. The count is in game frames (15 to the second, 900 to the game minute) and it starts when the projectile is launched.

Only a projectile steered by the homing flight model reads its fuse at all. A projectile whose [`ROT`](/keys/rot/#scope-bullettype) is zero never checks it, so the setting changes nothing on an unguided shot. The impact and pass-by paths in the caution below are what end an unguided shot instead, and none of them reads the fuse.

A shot at an aircraft is armed with zero whatever the section says.

```ini title="rules.ini"
[MYSEEKER] ; a BulletType, registered by a weapon naming it as its Projectile
Image=MISSILE
ROT=5
Arm=30 ; the fuse cannot trip for the first two seconds of flight
```

:::caution[Arming does not hold off the other detonation paths]
Only the fuse is held shut. A projectile still detonates while arming under **Any of:**

- it reaches its target;
- it drops to ground level;
- it runs into a wall or another tall overlay;
- it crosses a bridge deck;
- it passes within half a cell of a unit that does not belong to the firer or to one of the firer's allies;
- it has spent the flight distance a [`Ranged=yes`](/keys/ranged/) projectile is given;
- **All of:** it is marked [`AA=yes`](/keys/aa/), and it comes within half a cell of the aircraft or airborne jumpjet it is chasing;
- it stops gaining on its target.

A very large figure therefore does not make a projectile fly forever; it only stops the proximity trip from ever being the reason it goes off.
:::
