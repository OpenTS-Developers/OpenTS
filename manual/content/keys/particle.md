---
key: Particle
summary: The particle system released where the warhead detonates.
see_also: [Webby, WebRadius, BehavesLike, HoldsWhat]
when_omitted:
  kind: value
  value: none
---

An ordinary blast creates a system of the named type at the impact and has it release one particle. A type whose [`BehavesLike=Gas`](/keys/behaveslike/#scope-particlesystemtype) is handled differently: no system is created for the blast, and the particle is released into the scenario's shared gas cloud instead. An ordinary system that [holds nothing](/keys/holdswhat/) releases nothing; on the gas path what is released comes from the shared cloud's own type, so the named type's `HoldsWhat` is not read at all.

```ini title="rules.ini"
[MyGasWH] ; example WarheadType
Particle=MyGasSys ; example ParticleSystemType
```

A [`Webby=yes`](/keys/webby/) warhead uses the setting on a different path: it releases one particle in every cell the web covers rather than one at the impact, and the gas substitution above is not made there.

A name no particle system declares is registered as a new system rather than refused.

:::danger[A web warhead without this setting crashes]
The web path builds its per-cell system without first checking that a type was named. A [`Webby=yes`](/keys/webby/) warhead that leaves `Particle` unset crashes the game the first time a shot with it detonates. Ordinary blasts test for the missing type and simply release nothing. A negative [`WebRadius`](/keys/webradius/) leaves the per-cell loops with nothing to run, so no system is built and nothing crashes.
:::
