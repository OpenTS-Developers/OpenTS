---
key: Inaccurate
summary: Scatters the aim point of an arcing shot before its arc is solved.
see_also: [Arcing, BallisticScatter]
when_omitted:
  kind: value
  value: "no"
---

The scatter reaches only a projectile that is also [`Arcing=yes`](/keys/arcing/). The offset from the barrel to the predicted target position is moved in a random direction by between half of [`[CombatDamage] BallisticScatter`](/keys/ballisticscatter/) and all of it. That is half a cell to a whole cell at the engine default. The amount is fixed: a shot at maximum range is displaced by exactly as much as a point-blank one. The ballistic arc is then solved to the displaced point. The shot therefore lands short, long or wide by that much, and the firer never corrects for it. The blast can still be moved back: a shot that is not [`Airburst=yes`](/keys/airburst/) and goes off as it lands within a cell and a half of its target is moved onto the target, as [where the blast lands](/systems/projectile-flight/#where-the-blast-lands) explains. The scatter at the engine default stays inside that reach.

```ini title="rules.ini"
[MYARTILLERYSHELL] ; a BulletType, registered by a weapon naming it as its Projectile
Image=120MM
Arcing=yes
Inaccurate=yes
```

Two smaller effects do not need `Arcing`. A projectile that goes off within 32 leptons of its target's center is normally moved onto it; an inaccurate one skips that move. The [other moves onto the target](/systems/projectile-flight/#where-the-blast-lands) still apply to it, and the one that follows reaches 42 leptons, or 128 for a target in the air. The skip therefore changes where the blast lands only with an [`EMEffect=yes`](/keys/emeffect/) warhead or a [`Splits=yes`](/keys/splits/) projectile, which that later move does not apply to. And the anti-armor and anti-infantry ratings the engine works out for an object with the weapon are halved. Those ratings are read when the AI decides where to post a unit: one with no rating at all waits in the middle of its base, and one that can fight heads for the edge.

A shot fired from a platform that is moving is separately marked inaccurate for that one shot, but nothing live reads that per-shot mark. Only the setting on the projectile type is read.
