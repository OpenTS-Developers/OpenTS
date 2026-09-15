---
key: Lobber
summary: Makes the weapon always throw its shot on the high arc rather than the flat one.
see_also: ["Arcing", "Projectile", "Speed"]
when_omitted:
  kind: value
  value: "no"
---

A ballistic shot has two launch angles that reach the same point: a flat one and a steep one. The flag picks the steep one every time. Without it the steep angle is taken only when the target sits high enough overhead that the horizontal distance to it is smaller than the height difference. That is the case where a flat shot would bury itself in the ground between.

```ini title="rules.ini"
[MyMortar] ; example WeaponType
Lobber=yes
Projectile=MyShell ; a BulletType, registered by a weapon naming it as its Projectile

[MyShell] ; example BulletType
Arcing=yes
ROT=0
```

The choice reaches the shot itself only when the weapon's [`Projectile=`](/keys/projectile/) sets [`Arcing=yes`](/keys/arcing/); anything else has its pitch decided by other means and flies the same way either way. The barrel a turreted object elevates is aimed by the same choice whatever the projectile does, so setting the flag on a weapon with a flat projectile still points the barrel skyward.

The barrel elevation is always solved for the weapon in the first slot, so a second-slot lobber elevates the barrel as the first weapon dictates. The shot itself uses the flag of the slot that fired.

When no launch angle reaches at all, the shot's [`Speed=`](/keys/speed/#scope-weapontype) is too low for the distance and the barrel falls back to a fixed elevation. An arcing shot is abandoned before it leaves, costing neither ammunition nor a reload.
