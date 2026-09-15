---
key: DamageParticleSystems
summary: The particle systems a damaged object gives off, sorted into sparks and smoke by their own behavior.
see_also: [DamageSmokeOffset, ConditionYellow, ConditionRed, ConditionYellowSparkingProbability, ConditionRedSparkingProbability, Cyborg]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
DamageParticleSystems=MYSPARKSYS,MYSMOKESYS ; ParticleSystemTypes registered in [ParticleSystems]
DamageSmokeOffset=0,0,90
```

The list is one pool that the engine splits by each entry's [`BehavesLike`](/keys/behaveslike/). Only two of those behaviors are ever drawn from it, and an entry with any other behavior is never used.

**Sparks** are the `Spark` entries. A damaged object checks for them on every one of its own passes, while its health ratio is below [`ConditionYellow`](/keys/conditionyellow/) and it is not more than ten leptons underground. Past that depth the plume is out of sight below the ground, so nothing attaches there and a running system is dropped. If it has no spark system running, one entry is picked from the spark entries at random and attached to the object at its center plus [`DamageSmokeOffset`](/keys/damagesmokeoffset/). The roll is [`ConditionRedSparkingProbability`](/keys/conditionredsparkingprobability/) once the ratio is below [`ConditionRed`](/keys/conditionred/), and [`ConditionYellowSparkingProbability`](/keys/conditionyellowsparkingprobability/) until then. Sparks are switched off for every InfantryType except a [`Cyborg=yes`](/keys/cyborg/) one, and switched on for everything else; there is no key for it.

**Smoke** are the `Smoke` entries, and they are attached on a damaging hit rather than on a pass. The attach happens under **All of:**

- the health ratio is at or below `ConditionYellow`
- the hit is the one that takes the object below half strength or below `ConditionRed`
- no smoke system is already running
- the object is not more than ten leptons underground

One smoke entry is picked at random and attached at the object's position plus `DamageSmokeOffset`. Repairing back above `ConditionYellow`, or sinking further than ten leptons underground, removes it again.

An object may run one spark system and one smoke system at a time, so a longer list widens the choice rather than the count.
