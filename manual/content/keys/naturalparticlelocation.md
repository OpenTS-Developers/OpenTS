---
key: NaturalParticleLocation
summary: Where a structure's continuous particle plume sits, relative to the structure.
see_also: [NaturalParticleSystem, DamageSmokeOffset, Cloakable]
when_omitted:
  kind: value
  value: 0,0,0
---

```ini title="rules.ini"
[MYSMOKESTACK] ; a BuildingType registered in [BuildingTypes]
NaturalParticleSystem=MYSTEAMSYS ; a ParticleSystemType registered in [ParticleSystems]
NaturalParticleLocation=0,-40,180 ; the chimney mouth
```

Three lepton components — X, Y and Z — added to the structure's own position to place its [`NaturalParticleSystem`](/keys/naturalparticlesystem/). Only a BuildingType reaches it.

The offset is also a switch. The plume is deleted when a cloaking structure turns fully transparent, and the branch that puts it back once the structure is visible again tests this offset instead of the system: at `0,0,0` the rebuild is skipped and the structure stays plumeless for the rest of the match.

:::danger[An offset without a system is a bad pairing]
Because the rebuild checks only the offset, a cloaking structure that carries a non-zero offset and no [`NaturalParticleSystem`](/keys/naturalparticlesystem/) builds a particle system out of no type at all the moment it finishes uncloaking. Reading that type's timings is the first thing the new system does, so it crashes there.
:::

:::note[A value with fewer than three components reads as the default]
`NaturalParticleLocation=0,-40` is short of the three components a location needs, so the key reads as its default and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
