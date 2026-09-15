---
key: Webby
summary: The warhead entangles the infantry it catches instead of damaging them.
see_also: [WebDuration, WebDurationVariation, WebRadius, Particle, IsWebImmune, "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

A projectile with such a warhead replaces its blast with a web. Every cell within [`WebRadius`](/keys/webradius/) of the impact releases one particle from the warhead's [`Particle`](/keys/particle/) system and hands zero damage to each object standing in it. No blast damage is worked out at all, so [`Verses`](/keys/verses/), [`Spread`](/keys/spread/#scope-warheadtype) and the crater roll play no part in what a web does. The impact animation and the flash still play as they would for any other shot.

```ini title="rules.ini"
[MyWebWH] ; example WarheadType
Webby=yes
Particle=MyWebSys ; example ParticleSystemType
WebDuration=600
WebRadius=2
```

An infantryman without [`IsWebImmune=yes`](/keys/iswebimmune/) is pinned struggling for the duration and takes nothing; that page covers what an immune soldier gets instead. Anything else caught in the covered cells takes the zero-damage hit and is otherwise untouched.

The flag has one effect on ordinary explosions as well. A blast dealing zero damage is normally abandoned before anything around it is examined, and a web warhead's blast is carried through regardless.

Weapon choice reads the flag too: [which weapon the score assumes](/systems/target-selection/#which-weapon-the-score-assumes) covers how an object with a web weapon in one slot picks between the two, and the hazard of leaving the other slot empty.

:::caution[The flag gates the other three web settings]
[`WebDuration`](/keys/webduration/), [`WebDurationVariation`](/keys/webdurationvariation/) and [`WebRadius`](/keys/webradius/) are read only while this one is on, and they are read after it within the same section. A section that sets them without setting this flag leaves all three at whatever they already held.
:::

:::danger[A web warhead with no particle system crashes]
The per-cell particle release is made without checking that [`Particle`](/keys/particle/) named a system. Setting this flag on a warhead that has none crashes the game the first time a shot with it detonates. A negative [`WebRadius`](/keys/webradius/) leaves the per-cell loops with nothing to run, so no system is built and nothing crashes.
:::
