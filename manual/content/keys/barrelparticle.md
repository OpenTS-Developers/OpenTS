---
key: BarrelParticle
summary: The particle system released by an exploding overlay, on one blast in four.
see_also: [AmmoCrateDamage, BarrelExplode, BarrelDebris, Explodes]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
BarrelParticle=MyGraySmokeSys ; a ParticleSystemType registered in [ParticleSystems]
```

:::danger[An unset system crashes the game on the roll that reaches it]
Building the system reads the named type before checking that one was named. Because only one blast in four gets that far, an unset value can survive several explosions and then crash the game on the next.
:::

On a twenty-five percent roll a system of this type is created at the coordinate handed to the blast, a point inside the cell the overlay stood in rather than its center. It is immediately told to release its held particle, so the smoke starts on the same frame as the blast rather than on the system's own first spawn. [`AmmoCrateDamage`](/keys/ammocratedamage/) covers the rest of the explosion.

The system has no source object, so it is anchored to the cell and drifts on its own type's settings alone.
