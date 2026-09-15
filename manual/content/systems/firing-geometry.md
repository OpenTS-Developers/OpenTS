---
title: Firing geometry and beam weapons
summary: "Settles where an object's shots and firing effects appear, in what order one shot produces them, and which of them hold the weapon shut until they burn out."
category: weapons-projectiles
keys:
  - AmbientDamage
  - Anim
  - AttachedParticleSystem
  - BarrelAnimIsVoxel
  - Burst
  - BurstDelay0
  - BurstDelay1
  - BurstDelay2
  - BurstDelay3
  - Charges
  - Elite
  - FiringSyncFrame1
  - IsBigLaser
  - IsLaser
  - IsRailgun
  - IsSonic
  - LaserDuration
  - LaserInnerColor
  - LaserOuterColor
  - LaserOuterSpread
  - Lobber
  - PBarrelLength
  - PBarrelThickness
  - Primary
  - PrimaryFireFLH
  - PrimaryFirePixelOffset
  - RailgunDamageRadius
  - Report
  - ROF
  - SBarrelLength
  - SBarrelThickness
  - Secondary
  - SecondaryFireFLH
  - SecondaryFirePixelOffset
  - TurretOffset
  - UseFireParticles
  - UseSparkParticles
  - VoxelBarrelOffsetToBarrelEnd
related:
  - type: system
    id: projectile-flight
  - type: system
    id: particle-systems
  - type: system
    id: target-selection
  - type: system
    id: veterancy
---

One routine turns a decision to shoot into the whole shot: the projectile, the beam or wave that goes with it, the muzzle flash, the report, the recoil, the round spent and the delay before the next one. It runs in a fixed order. Several of the things it produces are anchored at different points on the artwork, and several are read from a different weapon slot. That is how a beam and the shell it accompanies can leave from opposite sides of the same gun.

[Projectile flight and impact](/systems/projectile-flight/) owns what the shot does once it is in the air. This page owns the moment of firing.

## Weapon slots in brief

An object type has three weapon slots. [`Primary=`](/keys/primary/) and [`Secondary=`](/keys/secondary/) fill the first two; [`Elite=`](/keys/elite/) fills a third that is never fired on its own. Each slot holds more than a weapon: it also holds the firing offset, barrel length and barrel thickness that weapon uses, and those come from the art entry rather than from the rules.

Two substitutions replace a whole slot, offsets included, and both are settled every time the slot is used rather than once.

- An object that has reached elite [rank](/systems/veterancy/#the-elite-weapon) reads the third slot in place of the first. The offsets travel with the slot, and the third slot is filled from the first weapon's [`PrimaryFireFLH`](/keys/primaryfireflh/), [`PBarrelLength`](/keys/pbarrellength/) and [`PBarrelThickness`](/keys/pbarrelthickness/). An elite weapon therefore fires from the same point on the artwork as the weapon it replaces.
- A structure with anything plugged into an upgrade slot takes the requested slot from the first plugged-in type that supplies a weapon there. It also takes that type's firing offsets, not its own.

Throughout this page **the firing slot** means whichever slot the shot was ordered from, after those substitutions. **The first slot** means slot zero after them, which is the elite weapon on an elite object.

An object type names its weapons in rules.ini, and the offsets those weapons fire from are written against the type's image in art.ini. This example is abridged to the assignments this page turns on:

```ini title="rules.ini"
[GunTower]
Primary=GunTowerGun    ; the weapon the first slot fires
Secondary=GunTowerBeam ; the weapon the second slot fires

[GunTowerGun]
Burst=2                ; alternates the shot between the two sides of the gun
```

```ini title="art.ini"
[GunTower]
PrimaryFireFLH=120,20,80   ; forward, lateral and height of the first slot's mounting
PBarrelLength=40           ; how far that slot's muzzle runs past the mounting
PBarrelThickness=6         ; how far that muzzle sits above it
SecondaryFireFLH=140,0,60 ; the second slot's mounting, in the same three parts
SBarrelLength=40
SBarrelThickness=6
```

## The shot, step by step

The order matters because a step that fails ends the shot, and everything below it is skipped. The numbered steps below all belong to one call.

1. The firing slot's weapon and projectile are fetched. A slot with no weapon does nothing at all.
2. A warhead whose [`LimpetFactor`](/keys/limpetfactor/) is `1` or more diverts here whenever the target is a structure, vehicle, infantry or aircraft. The mark is clamped on and the firer removes itself. A target this house has already marked ends the shot here too, leaving the firer standing. Either way no shot is created.
3. The shot is refused outright where the effect this weapon would produce is still alive (see [below](#effects-that-hold-the-weapon-shut)).
4. The barrel is given its desired elevation.
5. The damage the shot will deal is worked out. An [`IsSonic=yes`](/keys/issonic/) or [`UseFireParticles=yes`](/keys/usefireparticles/) weapon has none at all. Anything else deals the weapon's [`Damage`](/keys/damage/#scope-weapontype) scaled by the house's and the object's firepower bias, and by the veteran firepower ability.
6. The projectile is created and [launched from the mounting](/systems/projectile-flight/#what-the-shot-leaves-with). A weapon that also draws a beam or a wave still creates its projectile here. A shot that finds no ballistic solution, or that cannot be placed at the mounting, ends here: nothing below runs, and no round is spent or reload started.
7. A turreted object goes into recoil.
8. A shot from a weapon whose slot has a barrel length above zero is given [two flight turns](/systems/projectile-flight/#the-turn) immediately, so that it appears past the muzzle rather than inside it. An [`IsLaser=yes`](/keys/islaser/) weapon and an [`Inviso=yes`](/keys/inviso/) projectile are both excluded. Those two turns are ordinary ones, so a shot that would detonate on them does.
9. The fire, spark and railgun effects are spawned at the muzzle, each only where its own hold is empty. The railgun's beam is walked and its damage dealt at this point, before anything below.
10. The burst counter advances, the reload delay is set from it, and the counter wraps.
11. The weapon's [`Report`](/keys/report/) sound plays from the mounting, or from the firer's own center when the projectile is [`Dropping=yes`](/keys/dropping/).
12. The firing animation from the weapon's [`Anim`](/keys/anim/) list is created at the muzzle. It is attached to the firer, unless the firer is a structure, which gets a depth adjustment instead. A list of 8, 16, 32 or 64 entries is indexed by the gun heading rounded to that many directions. The list is rotated so that its first entry belongs to a gun facing north-west; on an eight-entry list the second entry is north, the third north-east, and so on. A list of any other length always uses its first entry.
13. The sonic wave is created, from the muzzle to the target.
14. The laser beam is drawn, from the muzzle to the target. On a structure this also stops the turret animation dead. Where the structure is down to its last round it discharges its turret as well; [`Charges`](/keys/charges/) covers what that costs a charging weapon.
15. A round of ammunition is spent, and a firer standing hidden in shroud or fog reveals the ground around itself to the target's owner where that owner is under player control.

The reload delay is set at step 10, so the effects at step 9 are already alive when it is worked out. That is what lets the effect weapons take a different delay from everything else.

## The mounting and the muzzle

Two distinct points are computed for every shot, and they move differently as the gun aims.

The **mounting** is where the projectile is created and where the firing solution is measured from. It is built by taking a frame aligned with the gun heading, then displacing it by the firing slot's own offset, with [`TurretOffset`](/keys/turretoffset/) added to the forward component. The barrel is deliberately not part of it, so the mounting holds still while the gun elevates.

The **muzzle** is where the fire animation, the laser beam, the sonic wave and the attached particle systems appear. It carries on from the same displacement. It adds the slot's barrel thickness upward, then runs out along the pitched barrel by its length.

The gun heading is not the same thing for every kind of object, and neither is the frame the two points are measured in.

| Firer | Gun heading | Mounting | Muzzle |
| --- | --- | --- | --- |
| Vehicle with a turret | The turret facing | Measured inside the transform the body is drawn with, so ground slope and body tilt move it | Measured upright, so slope and tilt do not move it |
| Vehicle without a turret | The body facing | As above | As above |
| Aircraft | The turret facing, whether or not it has a turret | As above | As above |
| Infantry | The body facing | The same point as the muzzle, so barrel length and thickness move the projectile too | Measured upright |
| Structure with a turret | The turret facing | Measured upright | Measured upright |
| Structure without a turret | The direction from its center to what it is shooting at | Measured upright | Measured upright |

A structure has two further layers on top of that, tested in this order for both points alike:

1. A [`PrimaryFirePixelOffset`](/keys/primaryfirepixeloffset/) other than `65535,65535` replaces both outright, whichever slot is firing. The offset is a screen offset projected back onto the ground, and nothing below is read.
2. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) structure takes both from [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) run through the barrel's own placement. The two points then coincide at the end of the barrel, and both rise as it elevates.
3. Otherwise the ordinary transform applies, with the turret animation's own screen offset added when the turret is drawn as a voxel.

The barrel elevation itself is decided per firer as well. A structure whose first weapon is [`IsLaser=yes`](/keys/islaser/) points its barrel straight at the aim point rather than solving anything. An EM pulse cannon solves a ballistic arc, and retries a quarter faster when the first solve fails. Every other object solves the arc for its first weapon, which is why [`Lobber`](/keys/lobber/) in the second slot does not raise the barrel.

### How a burst alternates muzzles

A burst counter runs from zero to one less than the weapon's [`Burst`](/keys/burst/), and the geometry reads it as well as the reload. The lateral component of the firing offset is negated whenever the counter is odd, which is what gives a burst weapon a pair of muzzles either side of the centerline. A structure with a voxel barrel alternates through its barrel offset instead, and centers the third shot and any after it.

:::caution[A burst weapon's beam and its shell leave from opposite muzzles]
The counter advances at step 10 of the sequence above, and the parts of a shot are split either side of that. The projectile, the railgun beam and the attached particle systems are placed with the counter as it stood for this shot. The firing animation, the sonic wave and the laser beam are placed with the counter as it now stands, which is the next shot's. On a `Burst=2` weapon those are always the two opposite muzzles, so the flash and the beam appear on the far side of the gun from the shell. A structure with a voxel barrel is displaced the other way, because its barrel lookup subtracts one from the counter to compensate. There the animation, wave and beam land on the same barrel as the projectile, and the projectile's own effects land on the previous shot's barrel.

Only a weapon whose `Burst` is above one and whose firing offset has a non-zero lateral component is affected. Everything else places both groups at the same point.
:::

## What each part of a shot reads

Several parts of a shot are read from the first slot rather than from the slot that fired, which matters as soon as a beam or wave weapon is put in the second slot. The table gathers where each part is anchored and which slot supplies its settings.

| Part of the shot | Anchored at | Read from |
| --- | --- | --- |
| The projectile, its damage, warhead and [projectile range](/keys/projectilerange/) | The mounting | The firing slot |
| The report sound and the firing animation | The mounting and the muzzle respectively | The firing slot |
| The fire and spark particle systems | The muzzle | The firing slot |
| The railgun beam, its ambient damage, warhead and particle system | The muzzle | The firing slot |
| The sonic wave's ambient damage and warhead | The muzzle at creation, then re-anchored each frame to the first slot's muzzle | The first slot |
| The laser beam's [`LaserInnerColor`](/keys/laserinnercolor/), [`LaserOuterColor`](/keys/laseroutercolor/), [`LaserOuterSpread`](/keys/laserouterspread/) and [`LaserDuration`](/keys/laserduration/) | The muzzle | The first slot |
| The barrel elevation and whether it is lobbed | — | The first slot |
| The reload delay | — | The firing slot |

A sonic wave is created from the muzzle that fired it, and then follows the first slot's muzzle for the rest of its life. A second-slot sonic weapon on an object whose slots have different offsets therefore sees its wave jump on the frame after it is fired. A [`UseFireParticles=yes`](/keys/usefireparticles/) stream behaves the same way; [particle systems](/systems/particle-systems/#what-each-system-behavior-reads) owns the re-anchoring itself.

## Effects that hold the weapon shut

Four effects lock an object out of firing while they are alive: a fire particle stream, a spark particle spray, a railgun particle trace, and a sonic wave. The first three each occupy one of the object's [particle holds](/systems/particle-systems/#the-five-holds-an-object-keeps). The wave occupies a single slot of its own, so an object can have only one wave in flight.

The lock is applied at three separate places, and the three do not answer alike.

- Before the shot is considered at all, the slot that is **not** firing is examined. A live effect belonging to that slot's weapon reports the object as unable to fire at this target at all.
- The firing slot's own weapon is examined next, and a live effect there reports the object as still rearming.
- The firing routine tests the same four conditions again and returns without creating anything.

The tests ask whether the *hold* is occupied, not whether this weapon's own effect is the thing occupying it. That matters for one hold only: sparks from a damaged object and a spark weapon's spray share theirs, and [particle systems](/systems/particle-systems/#the-five-holds-an-object-keeps) records what that costs.

A vehicle has one further gate that is not about effects at all. [`FiringSyncFrame1`](/keys/firingsyncframe1/) ties the first round of a burst to the firing animation, and [`FiringSyncFrame2`](/keys/firingsyncframe2/) the second. The gate holds the weapon as rearming until the animation reaches the named frame, and then releases the round without checking the reload timer. Only the first slot is tied this way; a shot from the second slot is never held.

## The reload delay

The delay before the next shot is chosen by the first of these that applies, so a row further down is reached only when every row above it has been passed over.

| The firer and its weapon | Frames until the next shot |
| --- | --- |
| A structure whose [`Ammo`](/keys/ammo/) pool holds more than one round | 1 |
| The firing slot holds no weapon | 1 |
| The weapon is [`IsSonic=yes`](/keys/issonic/) (whether or not a wave is actually alive), or is a fire, spark or railgun weapon whose matching particle hold is occupied | Exactly [`ROF`](/keys/rof/) |
| The burst is not finished | The matching [`BurstDelay0`](/keys/burstdelay0/) to [`BurstDelay3`](/keys/burstdelay3/), or a random 3 to 5 where that entry is `-1` or the shot is past the fourth |
| Anything else | `ROF` scaled by the house's rate-of-fire bias, plus a random 0 to 2, and shortened by the veteran rate-of-fire ability |

The third row is why an effect weapon fires at the pace it does. It skips the bias, the burst gaps and the random padding alike, so `ROF` and the effect's own lifetime decide the firing rate and nothing else does. On a structure with a stock of rounds even `ROF` drops out, because the first row has already returned a single frame and only the effect's lifetime is left to pace it.
