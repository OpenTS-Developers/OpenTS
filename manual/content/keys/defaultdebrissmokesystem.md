---
key: DefaultDebrisSmokeSystem
summary: Parsed particle system name that the engine never uses.
no_effect: true
see_also: ["DefaultFirestormExplosionSystem", "DamageParticleSystems", "AttachedParticleSystem"]
when_omitted:
  kind: value
  value: none
---

Nothing asks for a debris smoke system by this name. The systems an object or a weapon gives off are named where they are produced: an object's own [`DamageParticleSystems`](/keys/damageparticlesystems/), a weapon's [`AttachedParticleSystem`](/keys/attachedparticlesystem/), a warhead's [`Particle`](/keys/particle/). None of them falls back to this section.

The section sets ten `Default…System` names, each parsed into its own place. Nine of them go unread: this one, [`DefaultFireStreamSystem`](/keys/defaultfirestreamsystem/), [`DefaultLargeGreySmokeSystem`](/keys/defaultlargegreysmokesystem/), [`DefaultLargeRedSmokeSystem`](/keys/defaultlargeredsmokesystem/), [`DefaultRepairParticleSystem`](/keys/defaultrepairparticlesystem/), [`DefaultSmallGreySmokeSystem`](/keys/defaultsmallgreysmokesystem/), [`DefaultSmallRedSmokeSystem`](/keys/defaultsmallredsmokesystem/), [`DefaultSparkSystem`](/keys/defaultsparksystem/) and [`DefaultTestParticleSystem`](/keys/defaulttestparticlesystem/). [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) is the only one the engine reaches for, and it is reached from one branch rather than as a fallback for the others.
