---
title: Particle systems
summary: "Gives each particle system one turn a frame, in which its behavior alone settles which of the settings written on its type are read and which are passed over."
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

A particle system owns particles. It creates them, ages them, moves them, replaces them and takes them off the map, and none of that happens to a particle except on the turn its holding system takes. One setting settles which of those things a given system does: [`BehavesLike`](/keys/behaveslike/), whose seven values pick the routine that runs and the handful of settings written beside it that the routine reads.

That is why a section with fifteen assignments can produce an effect answering to four of them. The two tables below are the whole of the division. Each names every setting its behavior reads. A setting missing from its row is read into the type, kept there, and never looked at again.

## Particle system types and particles in brief

Two type definitions are involved, and they are easy to confuse, because both have a key named `BehavesLike` and both accept the same seven names for it.

A **ParticleSystemType** describes an emitter: how often it puts particles out, how many, in what direction, and when it gives up. A **ParticleType** describes one puff, spark, flame or cloud: what it looks like, how long it lives, how it moves, and what it does to whatever it touches. [`HoldsWhat`](/keys/holdswhat/) is the link between them, naming the one particle type a system makes for itself. [Rules registration lists](/formats/rules-registries/) covers how the `[ParticleSystems]` and `[Particles]` lists are read and what an unknown name does.

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

The two `BehavesLike` settings are independent. [The system's](/keys/behaveslike/#scope-particlesystemtype) decides how particles are emitted and aimed; [the particle's](/keys/behaveslike/#scope-particletype) decides what each one does once it exists. Nothing requires them to agree, and a smoke system holding a spark particle is a legal pairing that behaves as the pairing implies rather than as either name alone suggests.

At runtime the system is an object standing on the map, and the particles it holds are objects too. Only the system takes a logic turn.

## What each system behavior reads

Each row is one value of the setting and names what a running system of that type reads. Every other setting in the section is parsed, stored, and passed over.

Four settings are read whatever the value says, the last two rows included, so the rows below name them only where a behavior reads them a second way. [`Lifetime`](/keys/lifetime/) counts down for every system alike. [`OneFrameLight`](/keys/oneframelight/) with [`LightSize`](/keys/lightsize/) and [`ParticleCap`](/keys/particlecap/) draw the glow a system of any behavior may cast.

| `BehavesLike` | Used keys |
| --- | --- |
| `Smoke` | [`HoldsWhat`](/keys/holdswhat/), [`SpawnFrames`](/keys/spawnframes/), [`Slowdown`](/keys/slowdown/), [`SpawnCutoff`](/keys/spawncutoff/), [`SpawnTranslucencyCutoff`](/keys/spawntranslucencycutoff/), [`SpawnRadius`](/keys/spawnradius/) |
| `Fire` | [`HoldsWhat`](/keys/holdswhat/), [`SpawnFrames`](/keys/spawnframes/) |
| `Spark` | [`HoldsWhat`](/keys/holdswhat/), [`SparkSpawnFrames`](/keys/sparkspawnframes/), [`SpawnSparkPercentage`](/keys/spawnsparkpercentage/), [`SpawnDirection`](/keys/spawndirection/), and [`ParticleCap`](/keys/particlecap/), [`LightSize`](/keys/lightsize/) and [`OneFrameLight`](/keys/oneframelight/) a second way |
| `Railgun` | [`HoldsWhat`](/keys/holdswhat/), [`ParticlesPerCoord`](/keys/particlespercoord/), [`SpiralRadius`](/keys/spiralradius/), [`SpiralDeltaPerCoord`](/keys/spiraldeltapercoord/), [`PositionPerturbationCoefficient`](/keys/positionperturbationcoefficient/), [`MovementPerturbationCoefficient`](/keys/movementperturbationcoefficient/), [`VelocityPerturbationCoefficient`](/keys/velocityperturbationcoefficient/), [`Laser`](/keys/laser/), [`LaserColor`](/keys/lasercolor/) |
| `Gas`, `WeakGas`, `Web` | nothing, [`HoldsWhat`](/keys/holdswhat/) included |
| A name outside the seven, or no name at all | nothing, [`HoldsWhat`](/keys/holdswhat/) included |

[`Spawns`](/keys/spawns/#scope-particlesystemtype) appears in no row, under any behavior.

Three things the table cannot show go with it.

A gas, weak gas or web system makes no particle under its own power, but whatever built the system usually asks it for one on the spot, and that request does read `HoldsWhat`. A warhead's [`Particle`](/keys/particle/), an exploding [`BarrelParticle`](/keys/barrelparticle/) overlay, and each cell of a web blast all work that way. Everything such a system holds afterwards arrives through a successor.

Successors are the system's business rather than the particle's, and the settings they read sit on the particle type. A gas, weak gas or web system replaces each expiring particle with one [`NextParticle`](/keys/nextparticle/) successor displaced by [`NextParticleOffset`](/keys/nextparticleoffset/). A smoke system replaces it with two, thrown to either side by the expiring particle's [`Radius`](/keys/radius/) and ignoring the offset. Fire, spark and railgun systems make none, which leaves all three of those settings inert on any type they hold.

Only two behaviors move the system itself. A smoke system rides along with the object it was attached to, where it has one and that object is not a structure. It stops emitting altogether while that object is inside a subterranean tube. A fire system re-anchors to the firer's muzzle and re-aims along its body facing, but only on frames where the firer is both holding a target and still turning. A spark, railgun, gas, weak gas or web system stays exactly where it was created, however far the object it belongs to travels.

## The turn

Particle systems take their turns at a fixed point in the logic frame, after every other object kind and before terrain, walking the list of systems from the end toward the start. Every behavior ages each particle it holds and then removes the ones that flagged themselves as finished. Four of them also create particles, and where that falls differs: a spark or railgun system creates before the aging pass, a smoke or fire system after it.

Particles have no turn of their own: a particle ages and moves only where the routine holding it reaches for it. A fire system runs the movement step on every particle it holds, including the ones expiring that frame. Gas, weak gas, smoke and web systems run it on all but those. Spark and railgun systems never run it at all, so a particle held by one of those two moves only in whatever way its own aging step moves it, whatever its own behavior would otherwise do about drifting.

## Ending a system

Asking a system to go does not remove it. The request only marks the system. That stops a smoke, fire or railgun system emitting, and leaves a spark system throwing bursts until its own [`SparkSpawnFrames`](/keys/sparkspawnframes/) run out. Either way the system stays on the map until the last particle it is holding has died, and only then does it leave. This is why a weapon whose shot is a particle effect is barred from firing again until that effect has burned out. It is also why a repaired vehicle can go on smoking for a while after the damage that started the plume was undone.

A system is marked under any of:

- its own behavior's condition is met: a smoke system's spawn interval passing [`SpawnCutoff`](/keys/spawncutoff/), a spark system's [`SparkSpawnFrames`](/keys/sparkspawnframes/) running out, a railgun system finishing the single pass that lays its trace, or a fire system finding it has no live firer left to follow;
- a positive [`Lifetime`](/keys/lifetime/) counts down to zero;
- the object the system was attached to is taken off the map;
- something outside asks it to go: a repair, a cloak, an order to move, a new target out of range, or the [Remove particle anim at...](/mapping/actions/taction-remove-particle-anim/) action. That action matches systems by the cell they are standing in at that moment.

`Gas`, `WeakGas` and `Web` have no condition of their own, so a system of one of those three, or of no behavior at all, goes only by one of the last three routes.

## The five holds an object keeps

A vehicle, aircraft, infantry or structure keeps five **holds**, each one a place for a single running system. A hold that is already full is why a second system of the same kind is not started.

| Hold | What claims it |
| --- | --- |
| Fire | the stream a [`UseFireParticles=yes`](/keys/usefireparticles/) weapon throws |
| Spark | the spray a [`UseSparkParticles=yes`](/keys/usesparkparticles/) weapon throws, and the sparks a damaged object gives off |
| Natural | a structure's [`NaturalParticleSystem`](/keys/naturalparticlesystem/) |
| Damage | the smoke a damaged object gives off |
| Railgun | the trace an [`IsRailgun=yes`](/keys/israilgun/) weapon lays |

The types themselves are named elsewhere: a weapon's by [`AttachedParticleSystem`](/keys/attachedparticlesystem/), a damaged object's by [`DamageParticleSystems`](/keys/damageparticlesystems/). That second assignment is one list split by each entry's behavior, into the sparks that fill the spark hold and the smoke that fills the damage hold.

:::caution[Damage sparks and a spark weapon share one hold]
Nothing separates them. A damaged object that has picked up a spark system from its [`DamageParticleSystems`](/keys/damageparticlesystems/) cannot fire a `UseSparkParticles=yes` weapon until that system has cleared, because the test that bars the shot asks only whether the spark hold is occupied. It runs the other way too: while a spark weapon's spray is alive, the object grows no damage sparks. The fire, natural, damage and railgun holds each answer to one claimant and are unaffected.
:::

:::danger[Deleting a structure's plume outright strands half of its particles]
Every other system on this page is marked and left to run itself out. A cloaking structure's plume is the exception: it is removed outright the moment the structure turns fully transparent, while it may still be holding particles. The removal frees the first particle in its list, drops the second from that list without freeing it, and repeats down the list. Roughly half the puffs the plume was holding are left behind with no system keeping them. Nothing ages, moves or removes them afterwards, and they go on being drawn where they stood for the rest of the scenario, a fresh patch of them for every cloak. [`NaturalParticleLocation`](/keys/naturalparticlelocation/) covers the rebuild on the way back out, which is guarded by the offset rather than by the system.
:::

## What each particle behavior reads

The same division applies one level down, on the ParticleType. [`MaxEC`](/keys/maxec/) is read whatever the value says and is left out of the table. So are `NextParticle`, `NextParticleOffset` and [`Radius`](/keys/radius/): all three answer to the holding system's row above rather than to any row here.

| `BehavesLike` | Used keys |
| --- | --- |
| `Gas` | [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype), [`Warhead`](/keys/warhead/#scope-particletype), [`WindEffect`](/keys/windeffect/), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `WeakGas` | everything `Gas` reads except [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype) and [`Warhead`](/keys/warhead/#scope-particletype) |
| `Smoke` | [`Velocity`](/keys/velocity/), [`Deacc`](/keys/deacc/), [`WindEffect`](/keys/windeffect/), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `Fire` | [`Velocity`](/keys/velocity/), [`Deacc`](/keys/deacc/), [`MaxDC`](/keys/maxdc/), [`Damage`](/keys/damage/#scope-particletype), [`Warhead`](/keys/warhead/#scope-particletype), [`FinalDamageState`](/keys/finaldamagestate/), [`Normalized`](/keys/normalized/#scope-particletype), [`Translucency`](/keys/translucency/#scope-particletype), [`Translucent25State`](/keys/translucent25state/), [`Translucent50State`](/keys/translucent50state/), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| `Spark` | [`XVelocity`](/keys/xvelocity/), [`YVelocity`](/keys/yvelocity/), [`MinZVelocity`](/keys/minzvelocity/), [`ZVelocityRange`](/keys/zvelocityrange/), [`ColorList`](/keys/colorlist/), [`ColorSpeed`](/keys/colorspeed/), [`StartColor1`](/keys/startcolor1/), [`StartColor2`](/keys/startcolor2/) |
| `Railgun` | [`Velocity`](/keys/velocity/), [`ColorList`](/keys/colorlist/), [`ColorSpeed`](/keys/colorspeed/), [`StartColor1`](/keys/startcolor1/), [`StartColor2`](/keys/startcolor2/) |
| `Web` | [`Warhead`](/keys/warhead/#scope-particletype), [`Translucency`](/keys/translucency/#scope-particletype), [`StartStateAI`](/keys/startstateai/), [`EndStateAI`](/keys/endstateai/), [`StateAIAdvance`](/keys/stateaiadvance/), [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) |
| A name outside the seven, or no name at all | [`Translucency`](/keys/translucency/#scope-particletype) |

[`StartFrame`](/keys/startframe/) and [`NumLoopFrames`](/keys/numloopframes/) appear in no row, under any behavior.

Two entries above can be misread. `Normalized` recomputes the interval between animation states as the particle is created, but only a `Fire` particle reads the recomputed figure. On every other behavior the setting is stored and the authored [`StateAIAdvance`](/keys/stateaiadvance/) is what runs. A `Web` particle does read its warhead, but always applies it at zero damage, so its own [`Damage`](/keys/damage/#scope-particletype) figure changes nothing.

Two things outside the section reach only part of the set. [`WindDirection`](/keys/winddirection/) picks one fixed offset for the whole map, and `Gas`, `WeakGas` and `Smoke` particles are the only ones that take it, with [`WindEffect`](/keys/windeffect/) scaling how often or how far. The vertical pull of [`Gravity`](/keys/gravity/) reaches `Gas`, `WeakGas` and `Spark` particles alone.

## Systems that no attachment holds

Four sites build a system that belongs to nothing at all. One comes from a warhead's [`Particle`](/keys/particle/) at each impact, and another from a [`Webby=yes`](/keys/webby/) warhead in every cell it covers. An exploding barrel's [`BarrelParticle`](/keys/barrelparticle/) fires on one blast in four, and the [Particle anim at...](/mapping/actions/taction-particle-anim/) action places one on the ground at a waypoint. None of those has a source object, so nothing removes it when anything is destroyed and nothing moves it once it is placed, whatever its behavior.

Two more sites build a system with a source but no hold: a piece of debris uses its [`AttachedSystem`](/keys/attachedsystem/), and a vehicle or aircraft killed by the firestorm warhead throws seven to nine copies of [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/). Both are marked when the object that made them goes.

Two systems are named by the engine rather than by any assignment, so a mod can only reach either through the section matching that exact name. `GasCloudSys` is the scenario's shared gas cloud. It is one system, built once as the scenario starts rather than once per blast, and every gas-behavior warhead particle is released into it instead of into a system of its own. That single system's turn is therefore what ages and drifts every such particle in the match, and its own `HoldsWhat` is what they are made of, whatever the warhead named. `GasPuffSys` is the puff a levitating vehicle leaves behind as it accelerates.

:::danger[Retiring the shared gas cloud crashes the next gas detonation]
Nothing rebuilds it. The one build happens as the scenario is set up, and the blast that releases a gas particle takes the shared cloud and puts a particle into it without first checking that it is still there. Two things can take it away mid-match. The first is a positive [`Lifetime`](/keys/lifetime/) on the `GasCloudSys` section, which is the one retirement route a gas system has. The second is a [Remove particle anim at...](/mapping/actions/taction-remove-particle-anim/) action whose waypoint lands on the cell the cloud stands in. The cloud is built at `10,10` and never moves. Either way it leaves once the last particle in it has died, and the next detonation with a gas-behavior [`Particle`](/keys/particle/) stops the game. Leave `Lifetime` off that section.
:::

Of the ten `Default…System` names the rules set, [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) is the only one that reaches a system that is ever built. The [`DefaultSparkSystem`](/keys/defaultsparksystem/) page covers the other nine as a group. [`HalfDamageSmokeLocation1`](/keys/halfdamagesmokelocation1/) and its two companions name damage-smoke offsets too, but [`DamageSmokeOffset`](/keys/damagesmokeoffset/) is the offset a damaged object uses.
