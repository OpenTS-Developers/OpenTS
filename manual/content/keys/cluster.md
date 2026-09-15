---
key: Cluster
summary: The number of blasts an ordinary projectile delivers, or the number of bomblets a splitting one releases.
see_also: [Splits, AirburstWeapon, RetargetAccuracy]
when_omitted:
  kind: value
  value: "1"
---

Which of the two the figure counts depends on [`Splits`](/keys/splits/).

An ordinary projectile applies its warhead this many times over. The first blast lands at the point of impact and every later one is thrown exactly one to two cells from that point in a random direction, the distance drawn afresh each time. Each blast deals the projectile's full damage, so the figure multiplies the damage the shot delivers rather than dividing it.

A splitting projectile applies its warhead once and releases this many bomblets of its [`AirburstWeapon`](/keys/airburstweapon/) instead. Where each bomblet then aims is [`RetargetAccuracy`](/keys/retargetaccuracy/)'s decision; the figure here only counts them.

```ini title="rules.ini"
[MYSHRAPNELSHELL] ; a BulletType, registered by a weapon naming it as its Projectile
Image=120MM
Arcing=yes
Cluster=4 ; four full-damage blasts, one on the target and three scattered around it
```

:::caution[Every blast picks its explosion from the ground under the first]
Whether a blast is close enough to the ground to raise a ground explosion at all is judged at that blast's own position. The land type that then chooses *which* explosion is read at the projectile's own point of impact instead, and that point does not move as the blasts scatter. The damage and the animation are placed correctly at each scattered point; only the choice of animation comes from where the projectile landed. A cluster falling across a shoreline therefore throws the same splash at every blast, whatever ground each one comes down on.
:::

:::caution[`Cluster=0` disarms an ordinary projectile]
The blasts are counted out one at a time, so a figure of `0` or below runs none of them. No damage is dealt, no explosion animation is played and no lighting flash is thrown; the projectile is simply removed on impact. A splitting projectile still delivers its own single blast at `0` and merely releases no bomblets.
:::
