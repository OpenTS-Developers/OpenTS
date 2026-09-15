---
key: ConditionRedSparkingProbability
summary: Chance each frame that a critically damaged object starts throwing sparks.
see_also: [ConditionYellowSparkingProbability, ConditionRed, DamageParticleSystems]
when_omitted:
  kind: value
  value: ".02"
---

The figure is a fraction from 0 to 1, drawn against once per frame, and it applies while the object's remaining strength is below [`ConditionRed`](/keys/conditionred/). Values above `1` are accepted and start a spark on every frame the draw is made; `0` never starts one. Between that fraction and [`ConditionYellow`](/keys/conditionyellow/) the draw uses [`ConditionYellowSparkingProbability`](/keys/conditionyellowsparkingprobability/) instead.

The draw is only made while the object has no spark system running, so at most one is attached at a time and the figure sets how quickly a fresh one starts rather than how many there are. It also needs the type's [`DamageParticleSystems`](/keys/damageparticlesystems/) to name at least one ParticleSystemType declaring [`BehavesLike=Spark`](/keys/behaveslike/#scope-particlesystemtype); one of those entries is picked at random and placed at the type's [`DamageSmokeOffset`](/keys/damagesmokeoffset/). An object sitting 10 leptons or more below ground level is skipped.

Infantry are eligible only when the type declares [`Cyborg=yes`](/keys/cyborg/). Vehicles, aircraft and structures always are, and no setting turns that off.
