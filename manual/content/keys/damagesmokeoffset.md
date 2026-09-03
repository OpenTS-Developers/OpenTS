---
key: DamageSmokeOffset
summary: Where a damaged object's spark and smoke systems are anchored, relative to the object.
see_also: [DamageParticleSystems, NaturalParticleLocation]
when_omitted:
  kind: value
  value: 0,0,0
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
DamageParticleSystems=MYSPARKSYS,MYSMOKESYS ; ParticleSystemTypes registered in [ParticleSystems]
DamageSmokeOffset=0,0,90 ; smoke rises from 90 leptons above the hull
```

Three lepton components — X, Y and Z — added to the object when one of its [`DamageParticleSystems`](/keys/damageparticlesystems/) is attached. The single offset serves both kinds. On a vehicle, an infantryman or an aircraft the two anchors are the same point, because an object's center and its position are one and the same. They part only on a structure, whose center sits at the middle of its footprint while its position is the reference cell — a horizontal difference, with no difference in height.

The offset is applied once, when the system is created. The system is attached to the object and follows it from there, so the offset fixes where the plume sits on the object rather than where it sits on the map.

:::note[A value with fewer than three components reads as the default]
`DamageSmokeOffset=0,90` is short of the three components an offset needs, so the key reads as its default and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
