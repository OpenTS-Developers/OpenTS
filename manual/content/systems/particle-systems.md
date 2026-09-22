---
title: Particle systems
summary: "How a particle system's behavior decides which settings take effect, how systems end, and which systems an object can hold at once."
category: weapons-projectiles
keys:
  - AttachedParticleSystem
  - AttachedSystem
  - BarrelParticle
  - BehavesLike
  - ColorList
  - ColorSpeed
  - Damage
  - DamageParticleSystems
  - DamageSmokeOffset
  - Deacc
  - DefaultFirestormExplosionSystem
  - DefaultSparkSystem
  - DeleteOnStateLimit
  - EndStateAI
  - FinalDamageState
  - Gravity
  - HalfDamageSmokeLocation1
  - HoldsWhat
  - IsRailgun
  - Laser
  - LaserColor
  - Lifetime
  - LightSize
  - MaxDC
  - MaxEC
  - MinZVelocity
  - MovementPerturbationCoefficient
  - NaturalParticleLocation
  - NaturalParticleSystem
  - NextParticle
  - NextParticleOffset
  - Normalized
  - NumLoopFrames
  - OneFrameLight
  - Particle
  - ParticleCap
  - ParticlesPerCoord
  - PositionPerturbationCoefficient
  - Radius
  - Slowdown
  - SparkSpawnFrames
  - SpawnCutoff
  - SpawnDirection
  - SpawnFrames
  - SpawnRadius
  - SpawnSparkPercentage
  - SpawnTranslucencyCutoff
  - Spawns
  - SpiralDeltaPerCoord
  - SpiralRadius
  - StartColor1
  - StartColor2
  - StartFrame
  - StartStateAI
  - StateAIAdvance
  - Translucency
  - Translucent25State
  - Translucent50State
  - UseFireParticles
  - UseSparkParticles
  - Velocity
  - VelocityPerturbationCoefficient
  - Warhead
  - Webby
  - WindDirection
  - WindEffect
  - XVelocity
  - YVelocity
  - ZVelocityRange
related:
  - type: format
    id: rules-registries
  - type: system
    id: cloaking
  - type: action
    id: TACTION_PARTICLE_ANIM
  - type: action
    id: TACTION_REMOVE_PARTICLE_ANIM
---

A particle system creates, ages, moves and removes the particles it holds. Particles take no turn of their own, so nothing happens to a particle except during its system's turn. The system's [`BehavesLike`](/keys/behaveslike/) value selects which of those steps run and which settings in the system's section are read.

Most settings apply to only some behaviors. A setting the behavior does not read is loaded and then ignored, so a section with fifteen assignments can produce an effect controlled by four of them. The two tables below list every setting each behavior reads.

## Particle system types and particles in brief

Two kinds of type definition work together. Both have a `BehavesLike` key, and both accept the same seven names, so they are easy to confuse.

A **ParticleSystemType** describes an emitter: how often it creates particles, how many, in which direction, and when it stops. A **ParticleType** describes one puff, spark, flame or cloud: how it looks, how long it lives, how it moves, and what it does to objects it touches. The system's [`HoldsWhat`](/keys/holdswhat/) names the one particle type it creates. [Rules registration lists](/formats/rules-registries/) covers how the `[ParticleSystems]` and `[Particles]` lists are read and what an unknown name does.

```ini title="rules.ini"
[ParticleSystems]
0=MYSMOKESYS

[Particles]
0=MYSMOKEPUFF

[MYSMOKESYS] ; the emitter
BehavesLike=Smoke
HoldsWhat=MYSMOKEPUFF
SpawnFrames=10
Slowdown=.0025
SpawnCutoff=15.0

[MYSMOKEPUFF] ; the thing emitted
Image=SGRYSMK1
BehavesLike=Smoke
MaxEC=80
Velocity=6
Deacc=.02
EndStateAI=20
```

The two `BehavesLike` values are independent. [The system's value](/keys/behaveslike/#scope-particlesystemtype) decides how particles are emitted and aimed. [The particle's value](/keys/behaveslike/#scope-particletype) decides what each particle does once it exists. They do not have to match. A smoke system can hold spark particles: the system decides when particles appear and whether they get successors, and each spark particle still ages and moves as a spark.

## What each system behavior reads

Each row lists the settings that a running system with that `BehavesLike` value reads on its turn. The system ignores every other setting in its section.

Four settings apply to every system, including those in the last two rows. [`Lifetime`](/keys/lifetime/) counts down for every system. [`OneFrameLight`](/keys/oneframelight/), [`LightSize`](/keys/lightsize/) and [`ParticleCap`](/keys/particlecap/) control the glow that a system of any behavior can cast. The table names these four only where a behavior also uses them in another way.

| `BehavesLike` | Used keys |
| --- | --- |
| `Smoke` | [`HoldsWhat`](/keys/holdswhat/), [`SpawnFrames`](/keys/spawnframes/), [`Slowdown`](/keys/slowdown/), [`SpawnCutoff`](/keys/spawncutoff/), [`SpawnTranslucencyCutoff`](/keys/spawntranslucencycutoff/), [`SpawnRadius`](/keys/spawnradius/) |
| `Fire` | [`HoldsWhat`](/keys/holdswhat/), [`SpawnFrames`](/keys/spawnframes/) |
| `Spark` | [`HoldsWhat`](/keys/holdswhat/), [`SparkSpawnFrames`](/keys/sparkspawnframes/), [`SpawnSparkPercentage`](/keys/spawnsparkpercentage/), [`SpawnDirection`](/keys/spawndirection/), plus [`ParticleCap`](/keys/particlecap/) for the size of each burst and [`LightSize`](/keys/lightsize/) for a flash on the first frame, which appears only when [`OneFrameLight`](/keys/oneframelight/) is off |
| `Railgun` | [`HoldsWhat`](/keys/holdswhat/), [`ParticlesPerCoord`](/keys/particlespercoord/), [`SpiralRadius`](/keys/spiralradius/), [`SpiralDeltaPerCoord`](/keys/spiraldeltapercoord/), [`PositionPerturbationCoefficient`](/keys/positionperturbationcoefficient/), [`MovementPerturbationCoefficient`](/keys/movementperturbationcoefficient/), [`VelocityPerturbationCoefficient`](/keys/velocityperturbationcoefficient/), [`Laser`](/keys/laser/), [`LaserColor`](/keys/lasercolor/) |
| `Gas`, `WeakGas`, `Web` | nothing, [`HoldsWhat`](/keys/holdswhat/) included |
| A name outside the seven, or no name at all | nothing, [`HoldsWhat`](/keys/holdswhat/) included |

No behavior reads [`Spawns`](/keys/spawns/#scope-particlesystemtype).

Some places that build a system ask it for one particle at once, whatever its behavior. That particle comes from `HoldsWhat`. A warhead's [`Particle`](/keys/particle/), an exploding barrel's [`BarrelParticle`](/keys/barrelparticle/) and each cell of a web blast work this way. For a gas, weak gas or web system, this first particle is the only one `HoldsWhat` ever supplies; every later particle is a successor. The [Particle anim at...](/mapping/actions/taction-particle-anim/) action asks for no particle, so a gas, weak gas or web system it places stays empty.

The system decides whether an expiring particle gets a successor, but the settings involved sit on the particle type:

- A gas, weak gas or web system replaces each expiring particle with one [`NextParticle`](/keys/nextparticle/) successor, displaced by [`NextParticleOffset`](/keys/nextparticleoffset/).
- A smoke system replaces it with two successors, thrown to either side by the expiring particle's [`Radius`](/keys/radius/). It ignores `NextParticleOffset`.
- Fire, spark and railgun systems create no successors, so all three settings have no effect on the types they hold.

Only smoke and fire systems move:

- A smoke system follows the object it is attached to, unless that object is a structure. It creates no particles while that object is traveling through a map tunnel.
- A fire system moves to the muzzle of its firer's first weapon slot and re-aims along the firer's body facing. It does so only on frames when the firer has a target and is still turning.

Spark, railgun, gas, weak gas and web systems stay where they were created, however far the object they belong to travels.

## The turn

Particle systems take their turns in each logic frame after animations, building lights, bullets, structures, aircraft, infantry and vehicles, and before terrain objects, voxel animations and waves.

On its turn, a system with one of the seven behaviors ages every particle it holds, then removes the particles that have finished. Smoke, fire, spark and railgun systems also create particles. Spark and railgun systems create them before aging; smoke and fire systems create them after.

A system with no recognized behavior does nothing on its turn. A particle put into it never ages, moves or expires, and the system stays on the map for as long as it holds that particle.

A particle moves only when its system runs the movement step for it:

- A fire system moves every particle it holds, including those expiring that frame.
- Gas, weak gas, smoke and web systems move every particle except those expiring that frame.
- Spark and railgun systems never run the movement step. In those systems, only `Spark` and `Railgun` particles move, because they move themselves as they age. Particles of other behaviors stay where they were created.

## Ending a system

Asking a system to end only marks it. A marked smoke, fire or railgun system stops creating particles. A marked spark system keeps throwing bursts until its [`SparkSpawnFrames`](/keys/sparkspawnframes/) run out, unless all its particles expire between two bursts first. Either way, the system stays on the map until its last particle has expired, and only then is it removed.

This delay has visible effects. A repaired vehicle can keep smoking for a while, because its damage smoke stops producing puffs but the existing puffs and their successors live out their time. A system also keeps its object's [hold](#the-five-holds-an-object-keeps) filled until it is removed.

A system is marked when any of these happens:

- Its behavior's end condition is met. A smoke system ends when its spawn interval passes [`SpawnCutoff`](/keys/spawncutoff/). A spark system with a positive [`SparkSpawnFrames`](/keys/sparkspawnframes/) ends when that count runs out. A railgun system ends after it lays its trace, which it does in a single turn. A fire system ends when it has no live firer to follow.
- A positive [`Lifetime`](/keys/lifetime/) counts down to zero.
- The object the system is attached to is taken off the map, changes owner, or starts to cloak.
- Damage smoke ends when its object is repaired above yellow health or goes underground.
- A fire stream ends when its firer is given a destination (a move order, or its own approach to a target) or a new target out of range.
- The [Remove particle anim at...](/mapping/actions/taction-remove-particle-anim/) action ends every system standing in the waypoint's cell at that moment.

`Gas`, `WeakGas` and `Web` have no end condition of their own, and neither does a spark system whose `SparkSpawnFrames` is 0. A system of that kind, or with no behavior, ends only through one of the other routes above.

## The five holds an object keeps

Each vehicle, aircraft, infantry and structure has five **holds**, and each hold keeps one running system. While a hold is filled, no second system of that kind starts for the object.

| Hold | What claims it |
| --- | --- |
| Fire | the stream a [`UseFireParticles=yes`](/keys/usefireparticles/) weapon throws |
| Spark | the spray a [`UseSparkParticles=yes`](/keys/usesparkparticles/) weapon throws, and the sparks a damaged object gives off (infantry only with [`Cyborg=yes`](/keys/cyborg/)) |
| Natural | a structure's [`NaturalParticleSystem`](/keys/naturalparticlesystem/) |
| Damage | the smoke a damaged object gives off |
| Railgun | the trace an [`IsRailgun=yes`](/keys/israilgun/) weapon lays |

A hold stays filled until its system is removed. An object therefore cannot fire a spark, fire or railgun weapon again until the previous effect has burned out. [Firing geometry](/systems/firing-geometry/) covers how a filled hold blocks each weapon slot.

The fire hold is the exception. A new destination (a move order, or the object's own approach to a target) or a new target out of range frees it at once. The old stream burns out on its own, and the object can start a new stream before it has finished.

The types come from other settings. A weapon names its system with [`AttachedParticleSystem`](/keys/attachedparticlesystem/). A damaged object picks from its [`DamageParticleSystems`](/keys/damageparticlesystems/) list: entries with the `Spark` behavior fill the spark hold, and entries with the `Smoke` behavior fill the damage hold.

:::caution[Damage sparks and a spark weapon share one hold]
A damaged object that has started a spark system from its [`DamageParticleSystems`](/keys/damageparticlesystems/) cannot fire a `UseSparkParticles=yes` weapon until that system is removed. The reverse also applies: while a spark weapon's spray is alive, the object produces no damage sparks. The fire, natural, damage and railgun holds each have a single source and are not affected.
:::

:::danger[Cloaking a structure strands part of its plume]
When a structure finishes cloaking, its natural plume is deleted at once instead of being marked. About half of the puffs the plume held are left behind. They stay on screen where they were, never age, move or expire, and remain for the rest of the scenario. Each cloak leaves another set. [`NaturalParticleLocation`](/keys/naturalparticlelocation/) covers how the plume is rebuilt when the structure uncloaks.
:::

## What each particle behavior reads

A particle's `BehavesLike` likewise decides which settings in its section are read. [`MaxEC`](/keys/maxec/) applies to every behavior and is left out of the table. Some particle settings are read by the holding system instead, so they depend on the system's behavior, not the particle's:

- [`NextParticle`](/keys/nextparticle/), [`NextParticleOffset`](/keys/nextparticleoffset/) and [`Radius`](/keys/radius/) control successors, as described under [what each system behavior reads](#what-each-system-behavior-reads).
- [`XVelocity`](/keys/xvelocity/), [`YVelocity`](/keys/yvelocity/), [`MinZVelocity`](/keys/minzvelocity/) and [`ZVelocityRange`](/keys/zvelocityrange/) are read by a `Spark` system, which uses them to throw each particle it creates, whatever that particle's behavior.

| `BehavesLike` | Used keys |
| --- | --- |
| `Gas` | [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype), [`Warhead`](/keys/warhead/#scope-particletype), [`WindEffect`](/keys/windeffect/), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `WeakGas` | everything `Gas` reads except [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype) and [`Warhead`](/keys/warhead/#scope-particletype) |
| `Smoke` | [`Velocity`](/keys/velocity/), [`Deacc`](/keys/deacc/), [`WindEffect`](/keys/windeffect/), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `Fire` | [`Velocity`](/keys/velocity/), [`Deacc`](/keys/deacc/), [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype), [`Warhead`](/keys/warhead/#scope-particletype), [`FinalDamageState`](/keys/finaldamagestate/), [`Normalized`](/keys/normalized/#scope-particletype), [`Translucency`](/keys/translucency/#scope-particletype), [`Translucent25State`](/keys/translucent25state/), [`Translucent50State`](/keys/translucent50state/), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `Spark` | [`ColorList`](/keys/colorlist/), [`ColorSpeed`](/keys/colorspeed/), [`StartColor1`](/keys/startcolor1/), [`StartColor2`](/keys/startcolor2/) |
| `Railgun` | [`Velocity`](/keys/velocity/), [`ColorList`](/keys/colorlist/), [`ColorSpeed`](/keys/colorspeed/), [`StartColor1`](/keys/startcolor1/), [`StartColor2`](/keys/startcolor2/) |
| `Web` | [`Warhead`](/keys/warhead/#scope-particletype), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| A name outside the seven, or no name at all | [`Translucency`](/keys/translucency/#scope-particletype) |

No behavior reads [`StartFrame`](/keys/startframe/) or [`NumLoopFrames`](/keys/numloopframes/).

`Normalized` recalculates the interval between animation states when a particle is created, but only `Fire` particles use the recalculated interval. Particles of every other behavior use [`StateAIAdvance`](/keys/stateaiadvance/) as written.

A `Web` particle applies its warhead at zero damage, so its [`Damage`](/keys/damage/#scope-particletype) has no effect.

Two global settings reach only some behaviors. [`WindDirection`](/keys/winddirection/) sets one wind direction for the whole map. Only `Gas`, `WeakGas` and `Smoke` particles drift with it. For `Gas` and `WeakGas` particles, [`WindEffect`](/keys/windeffect/) sets how often they take a wind step. For `Smoke` particles, it sets how far they move each frame. [`Gravity`](/keys/gravity/) affects only `Gas`, `WeakGas` and `Spark` particles.

## Systems that no attachment holds

These systems belong to no object:

- a warhead's [`Particle`](/keys/particle/), built at each impact, except that a `Gas` system joins the shared gas cloud described below;
- a [`Webby=yes`](/keys/webby/) warhead's `Particle`, built in every cell the web covers;
- an exploding barrel's [`BarrelParticle`](/keys/barrelparticle/), built on one blast in four;
- the system the [Particle anim at...](/mapping/actions/taction-particle-anim/) action places on the ground at a waypoint.

None of these has a source object, so no object's destruction ends them, and none moves after it is placed, whatever its behavior.

Two more are tied to an object but fill no hold. A piece of debris builds its [`AttachedSystem`](/keys/attachedsystem/). An aircraft, or a vehicle without [`DeathFrames`](/keys/deathframes/), killed by the firestorm warhead throws seven to nine copies of [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/); each of these picks a random direction for every burst and ignores `SpawnDirection`. Both kinds are marked when the object that made them is removed.

Two systems are named by the engine, not by any key. A mod configures them only through sections with these exact names:

- `GasCloudSys` is the scenario's shared gas cloud. The game builds it once as the scenario starts, at cell `10,10`, and it never moves. Every warhead whose `Particle` system has `BehavesLike=Gas` releases its particle into this cloud instead of building a system of its own. That particle comes from the cloud's `HoldsWhat`, not from the system the warhead names, and the cloud's turn ages and moves every such particle in the match. Veinhole monsters release their gas into it as well.
- `GasPuffSys` is the puff a levitating vehicle leaves behind as it accelerates. Each puff builds a new system.

:::danger[Keep the shared gas cloud alive]
The game never rebuilds `GasCloudSys`. Once it is removed, the next detonation of a warhead with a `Gas` [`Particle`](/keys/particle/) system crashes the game, and so does the next gas release from a veinhole monster. Two things can remove the cloud during a match:

- a positive [`Lifetime`](/keys/lifetime/) in the `[GasCloudSys]` section;
- a [Remove particle anim at...](/mapping/actions/taction-remove-particle-anim/) action whose waypoint is cell `10,10`.

In both cases the cloud is removed once its last particle has expired. Leave `Lifetime` unset in that section, and keep that action's waypoint off cell `10,10`.
:::

Of the ten `Default…System` settings, only [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) names a system the game ever builds. The [`DefaultSparkSystem`](/keys/defaultsparksystem/) page covers the other nine.

[`HalfDamageSmokeLocation1`](/keys/halfdamagesmokelocation1/), [`HalfDamageSmokeLocation2`](/keys/halfdamagesmokelocation2/) and a vehicle's [`HalfDamageSmokeLocation`](/keys/halfdamagesmokelocation/) are loaded but never used. Damage smoke and sparks start at [`DamageSmokeOffset`](/keys/damagesmokeoffset/).
