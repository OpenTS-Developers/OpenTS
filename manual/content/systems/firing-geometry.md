---
title: Firing geometry and beam weapons
summary: "Where a shot and its firing effects appear, the order in which one shot produces them, and which effects stop the weapon firing until they end."
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

One shot can produce a projectile, a laser beam or sonic wave, weapon particle effects, a firing animation, a sound, recoil, a spent round and a reload delay. These parts do not all appear at the same point on the firer, and some take their settings from a different weapon slot than the one that fired. As a result, a beam and the shell fired with it can leave from opposite sides of the same gun.

[Projectile flight and impact](/systems/projectile-flight/) covers the projectile after launch. This page covers the moment of firing.

## Weapon slots in brief

An object type has three weapon slots. [`Primary=`](/keys/primary/) fills the first, [`Secondary=`](/keys/secondary/) the second, and [`Elite=`](/keys/elite/) the third. The third slot is never fired directly; it only replaces the first slot on an elite object.

Each slot also holds a firing offset, a barrel length and a barrel thickness. These come from the type's image section in art.ini, not from rules.ini. The first slot uses [`PrimaryFireFLH`](/keys/primaryfireflh/), [`PBarrelLength`](/keys/pbarrellength/) and [`PBarrelThickness`](/keys/pbarrelthickness/). The second slot uses [`SecondaryFireFLH`](/keys/secondaryfireflh/), [`SBarrelLength`](/keys/sbarrellength/) and [`SBarrelThickness`](/keys/sbarrelthickness/). The third slot uses the first slot's three values.

Two substitutions can replace a whole slot, including its offsets. Both are checked each time the slot is used, so a promotion or a new upgrade applies from the next shot.

- An object at elite [rank](/systems/veterancy/#the-elite-weapon) uses the third slot in place of the first. Because the third slot takes the first slot's offsets, the elite weapon fires from the same point on the artwork. If the type sets no `Elite=` weapon, the first slot's weapon stays in use.
- A structure with upgrades plugged in takes each slot from the first upgrade type that has a weapon in that slot. It also takes that upgrade type's offsets for the slot. This replacement is checked before rank, so an upgrade's first-slot weapon also replaces the elite weapon.

On this page, **the firing slot** means the slot the shot was ordered from, after these substitutions. **The first slot** means slot one after these substitutions, so on an elite object it holds the elite weapon.

This abridged example shows where the settings go. The weapons are named in rules.ini, and their offsets are set in art.ini under the type's image, which is the type's ID unless `Image=` names another:

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
SecondaryFireFLH=140,0,60  ; the second slot's mounting, in the same three parts
SBarrelLength=40
SBarrelThickness=6
```

## The shot, step by step

One shot runs the steps below in order. A step that ends the shot skips every later step, so no round is spent and no reload delay is set.

1. The firing slot must hold a weapon. An empty slot fires nothing.
2. If the weapon's warhead has a [`LimpetFactor`](/keys/limpetfactor/) above `0` and the target is a vehicle, infantry, aircraft or structure, no projectile is fired. If the firer's house has not yet marked that target, the target is marked, the weapon's [`Report`](/keys/report/) sound plays, and the firer is taken off the map. If that house has already marked the target, the shot ends and the firer stays.
3. The shot ends if this weapon's effect from an earlier shot is still alive. [Effects that hold the weapon shut](#effects-that-hold-the-weapon-shut) lists these effects.
4. The barrel starts turning to the elevation chosen for this target.
5. The projectile's damage is set. An [`IsSonic=yes`](/keys/issonic/) or [`UseFireParticles=yes`](/keys/usefireparticles/) weapon gives its projectile no damage. For any other weapon, a positive [`Damage`](/keys/damage/#scope-weapontype) is multiplied by the house's firepower bias and the object's firepower bias, and raised further by the veteran firepower ability. A zero or negative `Damage` is used unchanged.
6. The projectile is created and [launched from the mounting](/systems/projectile-flight/#what-the-shot-leaves-with). A laser, railgun or sonic weapon also creates its projectile here. The shot ends if the projectile cannot be created, if no ballistic arc reaches the target, or if the projectile cannot be placed at the mounting.
7. A turreted object recoils.
8. If the firing slot's barrel length is above `0`, the projectile takes [two flight turns](/systems/projectile-flight/#the-turn) at once, so it starts two turns further along its path. A projectile with a [`ROT`](/keys/rot/#scope-bullettype) above `0` launches at a speed of 1, so these turns move it only a short way. [`IsLaser=yes`](/keys/islaser/) weapons and [`Inviso=yes`](/keys/inviso/) projectiles skip these turns. They are ordinary flight turns, so a projectile that would detonate during them does.
9. A `UseFireParticles=yes`, [`UseSparkParticles=yes`](/keys/usesparkparticles/) or [`IsRailgun=yes`](/keys/israilgun/) weapon starts its effect at the muzzle. The railgun beam deals its damage now, before any later step; `IsRailgun` covers what it hits.
10. The burst counter advances, the reload delay is set from the advanced counter, and the counter returns to `0` after the last round of the burst.
11. The weapon's [`Report`](/keys/report/) sound plays at the mounting. When the projectile is [`Dropping=yes`](/keys/dropping/), it plays at the firer's center.
12. The firing animation from the weapon's [`Anim`](/keys/anim/) list appears at the muzzle. It is attached to a firing vehicle, infantry or aircraft and moves with it. On a structure it stays where it appeared and is drawn with a depth adjustment.
13. An `IsSonic=yes` weapon creates its sonic wave, from the muzzle to the target.
14. An `IsLaser=yes` weapon draws its beam, from the muzzle to the target. On a structure, the beam also resets the turret's charge animation to its first frame. The structure then discharges its turret unless it has more than one round of [`Ammo`](/keys/ammo/) left, so every structure with the default unlimited `Ammo` discharges. [`Charges`](/keys/charges/) covers what that means for a charging weapon.
15. A round of ammunition is spent.
16. When the target is an object owned by a player-controlled house, a firer that stands in shroud or fog, or that the local player has not yet discovered, reveals the area within two cells of itself to that house. A shot at the ground reveals nothing. The local player's aircraft are exempt from the shroud and fog test.

The `Anim` list is indexed by the direction the object is firing. A list of 8, 16, 32 or 64 entries uses the entry for that direction, rounded to the list's number of directions. The first entry is for north-west, and the list continues clockwise, so on an eight-entry list the second entry is north and the third north-east. A list of any other length always uses its first entry.

Step 10 sets the reload delay after step 9 has started the weapon's effects. This lets the delay depend on whether an effect is alive; [The reload delay](#the-reload-delay) gives the result.

## The mounting and the muzzle

Every shot uses two points on the firer, and they move differently as the gun aims.

The **mounting** is where the projectile is created and the point its aim is calculated from. It is the firing slot's offset turned to the gun heading, with [`TurretOffset`](/keys/turretoffset/) added to the forward distance. It ignores the barrel, so it stays in place while the gun elevates.

The **muzzle** is where the firing animation, the laser beam, the sonic wave and the weapon particle effects appear. It starts from the same offset, rises by the slot's barrel thickness, then extends by the barrel length along the elevated barrel.

The gun heading, and whether the points tilt with the body, depend on the kind of firer:

| Firer | Gun heading | Mounting | Muzzle |
| --- | --- | --- | --- |
| Vehicle with a turret | The turret facing | Tilts with the body on slopes | Stays upright |
| Vehicle without a turret | The body facing | Tilts with the body on slopes | Stays upright |
| Aircraft | The turret facing, whether or not it has a turret | Tilts with the body | Stays upright |
| Infantry | The body facing | The same point as the muzzle, so barrel length and thickness move the projectile too | Stays upright |
| Structure with a turret | The turret facing | Stays upright | Stays upright |
| Structure without a turret | The direction from its center to its target | Stays upright | Stays upright |

A structure can replace both points. The engine tests these cases in order and uses the first that applies:

1. A structure with a [`PrimaryFirePixelOffset`](/keys/primaryfirepixeloffset/) other than `65535,65535` uses that screen offset, projected onto the ground, for both points and for every slot.
2. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) structure places both points at the end of its voxel barrel, from [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/). Both points rise as the barrel elevates.
3. Any other structure uses the points described above. When its turret is a voxel, the turret animation's screen offset is added to both.

The barrel's elevation is chosen from the first slot, whichever slot fires:

- A structure whose first-slot weapon has `IsLaser=yes` points its barrel straight at the aim point.
- An [`EMPulseCannon=yes`](/keys/empulsecannon/) structure solves a ballistic arc. If no arc reaches the target, it tries again with a launch speed a quarter higher.
- Every other object solves a ballistic arc for its first-slot weapon. The barrel takes the high arc when that weapon is [`Lobber=yes`](/keys/lobber/), or when the target stands higher above the firer than it is away horizontally. `Lobber` on the second slot does not raise the barrel. The projectile itself still follows its own weapon's `Lobber`.

### How a burst alternates muzzles

The burst counter runs from `0` up to one less than the weapon's [`Burst`](/keys/burst/). When the counter is odd, the lateral part of the firing offset is reversed. A burst weapon therefore fires alternately from two points on either side of the centerline. A structure with a voxel barrel alternates the lateral part of its barrel offset instead. The first shot of a burst uses one side and the second the other. The third and later shots come from the center.

:::caution[A burst weapon's beam and its shell leave from opposite muzzles]
Step 10 advances the burst counter partway through the shot, so the parts of one shot are placed with two different counter values:

- **This shot's counter:** the projectile, the report sound, and the fire, spark and railgun effects.
- **The next shot's counter:** the firing animation, the sonic wave and the laser beam.

The two groups appear on opposite sides of the gun when one counter value is odd and the other even. With an even `Burst` that is every shot. With an odd `Burst` it is every shot except the last of each burst.

A structure with a voxel barrel works the other way, because its muzzle uses the previous counter value, which cancels the advance. There the firing animation, sonic wave and laser beam appear at the projectile's barrel, while the fire, spark and railgun effects appear at the previous shot's barrel.

A weapon is affected only if its `Burst` is above `1` and its lateral offset is not zero. `PrimaryFirePixelOffset` places every part at one point.
:::

## What each part of a shot reads

Several parts of a shot take their settings from the first slot, not from the slot that fired. This matters when a beam or wave weapon is in the second slot, or when the two slots have different offsets.

| Part of the shot | Appears at | Settings from |
| --- | --- | --- |
| The projectile, its damage, warhead and [projectile range](/keys/projectilerange/) | The mounting | The firing slot |
| The report sound | The mounting | The firing slot |
| The firing animation | The muzzle | The firing slot |
| The fire and spark particle systems | The muzzle | The firing slot |
| The railgun beam, its [`AmbientDamage`](/keys/ambientdamage/), warhead and [`AttachedParticleSystem`](/keys/attachedparticlesystem/) | The muzzle | The firing slot |
| The sonic wave's `AmbientDamage` and warhead | The firing slot's muzzle when created, then the first slot's muzzle | The first slot |
| The laser beam's [`LaserInnerColor`](/keys/laserinnercolor/), [`LaserOuterColor`](/keys/laseroutercolor/), [`LaserOuterSpread`](/keys/laserouterspread/), [`LaserDuration`](/keys/laserduration/) and [`IsBigLaser`](/keys/isbiglaser/) | The firing slot's muzzle | The first slot |
| The barrel elevation, including the high arc from `Lobber` | Not placed | The first slot |
| The reload delay | Not placed | The firing slot |

A sonic wave starts at the muzzle of the slot that fired it. From the next frame it is redrawn from the first slot's muzzle. If the two slots have different offsets, a second-slot sonic wave therefore jumps one frame after it is fired. A [`UseFireParticles=yes`](/keys/usefireparticles/) stream also moves to the first slot's muzzle, but only on frames when the firer has a target and is still turning. [Particle systems](/systems/particle-systems/#what-each-system-behavior-reads) covers how it follows the firer.

## Effects that hold the weapon shut

Four weapon effects stop an object from firing while they are alive:

- the fire stream of a [`UseFireParticles=yes`](/keys/usefireparticles/) weapon;
- the spark spray of a [`UseSparkParticles=yes`](/keys/usesparkparticles/) weapon;
- the railgun trace of an [`IsRailgun=yes`](/keys/israilgun/) weapon;
- the sonic wave of an [`IsSonic=yes`](/keys/issonic/) weapon.

The first three each occupy one of the object's [particle holds](/systems/particle-systems/#the-five-holds-an-object-keeps). An object has room for one sonic wave, so it can have only one alive at a time.

The object checks these effects in three places, with a different result each time:

1. **The other slot.** If the weapon in the slot that is not firing has a live effect, the object reports that it cannot fire at this target at all.
2. **The firing slot.** If the firing slot's weapon has a live effect, the object is treated as still reloading.
3. **The shot itself.** Step 3 of the firing sequence repeats the firing slot's check and ends the shot.

Each check asks only whether the hold is occupied, not what occupies it. This matters only for the spark hold, which a damaged object's sparks share with a spark weapon's spray. [Particle systems](/systems/particle-systems/#the-five-holds-an-object-keeps) covers the result.

Vehicles have one more gate, unrelated to effects. [`FiringSyncFrame1`](/keys/firingsyncframe1/) times the first round of each burst to the firing animation, and [`FiringSyncFrame2`](/keys/firingsyncframe2/) times the second round. A weapon without a burst uses `FiringSyncFrame1` for every round. While the firing animation is playing, the round is treated as still reloading until the animation reaches the named frame. It then fires even if the reload delay has not run out. Only first-slot shots are timed this way.

## The reload delay

The delay before the next shot comes from the first row that applies:

| The firer and its weapon | Frames until the next shot |
| --- | --- |
| A structure that had more than one round of [`Ammo`](/keys/ammo/) when it fired | 1 |
| The weapon is [`IsSonic=yes`](/keys/issonic/), whether or not a wave is alive, or is a fire, spark or railgun weapon whose particle hold is occupied | Exactly [`ROF`](/keys/rof/) |
| The burst has rounds left | [`BurstDelay0`](/keys/burstdelay0/) after the first round, up to [`BurstDelay3`](/keys/burstdelay3/) after the fourth. A random 3 to 5 when that entry is `-1`, or after the fifth or a later round |
| Any other shot | `ROF` multiplied by the house's rate-of-fire bias, plus a random 0 to 2, then shortened by the veteran rate-of-fire ability |

The third row sets the pace of the effect weapons. It skips the house bias, the burst delays and the random extra frames, and the effect blocks firing until it ends. The next shot therefore waits until both `ROF` has passed and the effect has ended. A structure with more than one round left matches the first row instead, so only the effect's lifetime paces it.
