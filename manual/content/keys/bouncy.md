---
key: Bouncy
summary: Lets a ballistic projectile rebound off the surface it lands on instead of detonating there.
see_also: [Elasticity, Arcing]
when_omitted:
  kind: value
  value: "no"
---

Only a projectile with no rate of turn ever lands: one steered by the homing flight model detonates on arrival instead. Every landing already computes a rebound: the velocity is turned into the frame of reference of the ground slope, scaled by [`Elasticity`](/keys/elasticity/#scope-bullettype), reflected and turned back out again. That computation is what throws a projectile off a ramp downhill. A projectile without this setting is forced to detonate before that rebound can carry it anywhere.

A bouncy projectile detonates instead of rebounding when the cell the rebound happens in holds an object that does not belong to the firer or to one of the firer's allies. Bridges count as ground: a projectile that drops onto a deck or rises into its underside bounces off that surface rather than passing through.

Two limits end the flight regardless. The third bounce forces a detonation wherever the projectile came down. A projectile moving slower than ten leptons a frame and lying within ten leptons of the ground settles and detonates where it lies.

```ini title="rules.ini"
[MYGRENADE] ; a BulletType, registered by a weapon naming it as its Projectile
Image=DISCUS
Arcing=yes
Bouncy=yes
Elasticity=0.5 ; half its speed is kept on each rebound
```
