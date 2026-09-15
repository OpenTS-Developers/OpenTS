---
key: NaturalParticleSystem
summary: A particle system a structure runs continuously from the moment it appears.
see_also: [NaturalParticleLocation, DamageParticleSystems, Cloakable]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MYSMOKESTACK] ; a BuildingType registered in [BuildingTypes]
NaturalParticleSystem=MYSTEAMSYS ; a ParticleSystemType registered in [ParticleSystems]
NaturalParticleLocation=0,-40,180
```

This is the plume a structure gives off in good health, the counterpart to [`DamageParticleSystems`](/keys/damageparticlesystems/), which only appear once it is hurt. Only a BuildingType reaches it: an AircraftType, InfantryType or UnitType stores the setting and never reads it back.

The system is created the moment the structure appears on the map, at the structure's position offset by [`NaturalParticleLocation`](/keys/naturalparticlelocation/) and tied to the cell it stands on. A structure runs one of these at a time, and nothing removes it while the structure lives. The exception is cloaking: it deletes the plume the moment the structure turns fully transparent and rebuilds it only when the structure has finished becoming visible again.

That rebuild is guarded by the *location* rather than by this setting, so a structure whose `NaturalParticleLocation` is left at `0,0,0` never gets its plume back after its first cloak.
