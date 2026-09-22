---
title: Projectile flight and impact
summary: "Steps every projectile along one of two flight models each frame and settles where, and how many times, it detonates."
category: weapons-projectiles
keys:
  - AA
  - Acceleration
  - Airburst
  - AirburstWeapon
  - AnimHigh
  - AnimLow
  - AnimPalette
  - AnimRate
  - Arcing
  - Arm
  - BallisticScatter
  - Bouncy
  - Cluster
  - Degenerates
  - Dropping
  - Elasticity
  - Floater
  - Gravity
  - Height
  - High
  - HomingScatter
  - Image
  - Inaccurate
  - Inviso
  - MissileROTVar
  - MissileSpeedVar
  - Projectile
  - ProjectileRange
  - Proximity
  - Range
  - Ranged
  - RetargetAccuracy
  - ROT
  - Rotates
  - Shadow
  - Speed
  - Splits
  - Trailer
  - VeryHigh
  - Voxel
related:
  - type: system
    id: firing-geometry
  - type: system
    id: emp-pulse
  - type: system
    id: laser-fences
  - type: system
    id: particle-systems
  - type: system
    id: target-selection
---

A projectile carries figures from the weapon that fired it. The weapon supplies the damage, the warhead, the launch speed, the flight allowance, and the object credited with any kill. The projectile type decides how the shot travels, what ends its flight, and how many blasts it delivers.

[`ROT`](/keys/rot/#scope-bullettype) chooses between two flight models. A projectile with `ROT` above zero is steered: the missile autopilot turns it toward its target each frame and lifts it over the ground in the way. A projectile with `ROT=0` is ballistic: it follows the arc it was launched along, falls under gravity, and bounces off whatever it lands on. Most settings on this page affect only one of the two models.

[Firing geometry and beam weapons](/systems/firing-geometry/) owns where a shot appears and what is drawn alongside it. This page owns the shot itself, from the velocity it leaves with to the last blast it delivers.

## Projectile types in brief

A **BulletType** is a rules section describing one kind of shot. No list registers it. A weapon's [`Projectile=`](/keys/projectile/) names it, and a name the game has not seen before creates a new projectile type with default settings. A misspelled name therefore produces a projectile with none of the intended settings, and no error.

The weapon section holds how hard the shot hits, how far it can be fired, and how often. The projectile section holds how the shot flies, what it can be aimed at, and what its impact breaks into.

```ini title="rules.ini"
[MyRocketPod]      ; a WeaponType, registered by an object naming it as its Primary
Projectile=MYROCKET
Warhead=AP         ; a WarheadType registered in [Warheads]
Damage=60
Range=7
ProjectileRange=9

[MYROCKET]         ; a BulletType, registered by the weapon above naming it
Image=MISSILE      ; the art section its flight imagery is read from
ROT=5              ; above zero, so the missile autopilot flies it
Ranged=yes         ; spends the weapon's ProjectileRange as fuel
AA=yes
```

A projectile in flight occupies no cell. Nothing collides with it, and nothing that searches a cell for occupants finds it. It is always drawn with the airborne objects, whatever its height.

## What the shot leaves with

Five things are fixed at launch: the aim point, the direction, the pitch, the speed, and, on a steered projectile, the fuse.

The firer aims at its current target's position. When that target is a moving vehicle, the firer aims ahead of it, at where the vehicle will be when a shot at the speed of the firer's primary weapon arrives. On a projectile that is both [`Inaccurate=yes`](/keys/inaccurate/) and [`Arcing=yes`](/keys/arcing/), that aim point is then moved in a random direction by between half and all of [`BallisticScatter`](/keys/ballisticscatter/). The pitch and speed below are worked out for the moved point.

A steered projectile or a [`Dropping=yes`](/keys/dropping/) projectile leaves along the firer's gun heading, whichever way the target lies. Every other projectile leaves pointed at the aim point. The gun heading depends on the firer:

- a vehicle with a turret uses its turret facing;
- infantry and a vehicle without a turret use their body facing;
- an aircraft uses the heading its body is drawn at;
- a structure with a turret that is not a voxel uses its turret facing, rounded to one of 32 directions;
- any other structure, including one with a voxel turret, fires straight toward its target.

A lepton is the engine's distance unit. A cell is 256 leptons across and a terrain level is 104 leptons high, so 200 leptons is just under two terrain levels.

The pitch comes from the first of these rules that applies:

1. An [`Arcing=yes`](/keys/arcing/) projectile gets a ballistic arc to the aim point. The high arc is used when the weapon is [`Lobber=yes`](/keys/lobber/), or when the target stands higher above the firer than it is away horizontally. Otherwise the flat arc is used. When no arc at the launch speed reaches the aim point, the weapon does not fire.
2. A [`Voxel=yes`](/keys/voxel/) projectile leaves pointing straight up.
3. Any other projectile leaves almost level, about 1.4 degrees up, unless the aim point is more than 200 leptons above or below the weapon's mounting. Then it is pitched at the aim point, with the height difference reduced by 20 leptons first.
   - When that happens and the firer's target is a structure, the aim height becomes 200 leptons for each point of the structure's [`Height`](/keys/height/#scope-buildingtype). That height is counted from the map's zero height, not from the ground the structure stands on.

A ballistic projectile does not use the weapon's [`Speed=`](/keys/speed/#scope-weapontype). Once the rules are read, the engine replaces it with a speed worked out from the weapon's [`Range=`](/keys/range/) and gravity, fast enough to carry the shot the weapon's full range. At launch that speed is capped at half the distance to the aim point, so a point-blank shot leaves more slowly than a shot at full range. An arcing shot's arc is worked out at the capped speed.

A steered projectile leaves at 1 lepton a frame, whatever the distance, and works up to the weapon's `Speed=` in flight, as [steered flight](#steered-flight) describes.

Only a steered projectile uses a fuse. The fuse is set on the target's position at the moment of launch, not on the predicted aim point. It stays shut for the projectile's [`Arm`](/keys/arm/) delay, except that a shot at an aircraft is armed at once. The `Arm` page covers when the fuse trips.

An [`Inviso=yes`](/keys/inviso/) projectile skips the flight. It is placed directly on its target's position as it is launched. If a raised [firestorm wall](/systems/laser-fences/#projectiles) lies on the path, the wall consumes it instead.

:::caution[An aircraft replaces the launch velocity]
When an aircraft fires, it rewrites the projectile's velocity after launch. The rewrite depends on the projectile's exact `ROT`.

| `ROT` | Velocity after launch |
| --- | --- |
| `0` | Level, along the aircraft's heading, at the aircraft's current speed. The arc, pitch and speed worked out above are discarded, so an [`Arcing=yes`](/keys/arcing/) bomb dropped from the air does not follow its weapon's arc |
| `1` | Pointed straight at the target's center in three dimensions, at the full [`Speed=`](/keys/speed/#scope-weapontype) of the aircraft's primary weapon, whichever weapon fired. It does not start along the gun heading at 1 lepton a frame |
| `2` or more | Unchanged |
:::

## The turn

Every object takes its turn each frame in the order it was placed on the map, and a projectile is placed when it is fired. A projectile therefore normally moves after its target in the same frame, and sees the target where this frame left it. An object that leaves the map and returns, such as a unit leaving a transport, moves to the back of the order.

Each turn runs these steps in order:

1. A [`Dropping=yes`](/keys/dropping/) projectile is marked to detonate.
2. When [`AnimLow`](/keys/animlow/) or [`AnimHigh`](/keys/animhigh/) is not zero, the flight animation counts down toward its next frame.
3. On every third frame of the game clock, a [`Trailer`](/keys/trailer/) puff is left at the position the projectile is about to leave.
4. One of the two flight models moves the projectile, and may mark it to detonate.
5. A [`Ranged=yes`](/keys/ranged/) projectile subtracts the distance it just moved from its allowance, and is marked when the allowance runs out.
6. The projectile takes up its new position. If it now stands in a raised [firestorm wall](/systems/laser-fences/#projectiles) that does not let it through, the wall consumes it and the turn ends.
7. A projectile not already marked goes through three standing tests: a tall overlay in its cell, ground level, and the anti-aircraft proximity test. [What ends a flight](#what-ends-a-flight) lists them.
8. A steered projectile checks its fuse.
9. A projectile that is not marked, and whose fuse has not tripped, keeps flying. A [`Degenerates=yes`](/keys/degenerates/) projectile then loses 1 point of damage, down to a floor of 5. Any other projectile detonates and is removed.

A shot from a weapon with a barrel gets a head start: two of these turns run immediately during firing, so the shot appears past the muzzle, not inside it. [The firing sequence](/systems/firing-geometry/#the-shot-step-by-step) says which shots get the head start. Every other shot takes its first turn later in the same frame, once every object ahead of it in the turn order has moved. A head-start shot takes that turn as well, so it moves three times in the frame it is fired.

## Steered flight

The autopilot flies a projectile whose `ROT` is above zero. It homes on the target itself, at the point weapons aim at on it, and reads that point again every frame.

Each frame the autopilot moves the projectile's speed toward the weapon's figure, gaining [`Acceleration`](/keys/acceleration/#scope-bullettype) leptons a frame. A projectile already faster than that figure slows by half of `Acceleration` a frame, rounded down.

A new steered projectile is launching until it reaches that speed, unless the weapon's [`Speed=`](/keys/speed/#scope-weapontype) is `16` or more. `Speed=` is written on a 0-to-100 scale, and `16` converts to 40 leptons a frame. While launching, the projectile gains 1 lepton every other frame instead of `Acceleration`. It keeps its heading, and its pitch turns toward the target by less than one degree a frame.

After launching, the projectile may turn by up to its turn allowance each frame. The allowance is measured in 256ths of a full turn. It swings between `ROT` and `ROT` × (1 + 2 × [`MissileROTVar`](/keys/missilerotvar/)) over a 15-frame cycle, rounded down, which makes missiles weave. The cycle is offset for each projectile, so missiles fired together do not all weave in step. Within one cell of the target the allowance is multiplied by 1.5.

The heading turns toward the target by up to the full allowance. The pitch follows one of two branches. The projectile follows the terrain only when all of these hold:

- the target is not an aircraft;
- **Any of:** the projectile is [`Airburst=yes`](/keys/airburst/); the horizontal distance to the target is over three cells, or over six for a [`VeryHigh=yes`](/keys/veryhigh/) projectile;
- the turn allowance this frame is more than 1.

Otherwise the projectile dives.

A launching projectile's turn allowance is zero, so it never follows terrain. It always takes the dive branch, which is what turns its pitch slightly. At a `MissileROTVar` of `0.5` or less, a `ROT=1` projectile's allowance never exceeds 1, so it always dives. A `ROT` of 2 or more always passes the allowance test.

Terrain following looks six frames of travel ahead and holds the projectile above the ground there. The clearance is one terrain level for each whole cell still to the target, up to five levels. An `Airburst=yes` or `VeryHigh=yes` projectile holds ten levels instead. When the projectile is more than 20 leptons off that height, it is moved 18 leptons toward it each frame. Its pitch turns at half the allowance: toward a 45-degree climb when it is more than half a level too low, toward an 11-degree descent when it is more than half a level too high, and toward level flight otherwise.

In a dive, the pitch turns straight toward the target at just over half the allowance.

[`VeryHigh`](/keys/veryhigh/) raises the clearance and starts the dive farther out. [`Airburst`](/keys/airburst/) raises the clearance and keeps following the terrain all the way to the target, as long as the other two conditions hold.

The projectile has arrived when the distance still to go is no more than half its speed per frame, or when it is at ground level. That distance counts only a quarter of the height difference, or none of it for an `Airburst=yes` projectile. A projectile that arrives in the air is moved onto its homing point, unless it is `Airburst=yes`.

A steered projectile that stops gaining on its target detonates instead of circling it forever. After its first 60 frames past launching, the engine keeps a running average of how fast the projectile closes on its target. The flight ends when that average is at least 0 but below 1 lepton a frame. A projectile that is losing ground has a negative average, so this test does not stop it. `Airburst=yes` and `VeryHigh=yes` projectiles are exempt, because both are meant to stay above their targets.

A steered projectile that crosses the height of a bridge deck over a bridge cell, from above or below, ends its flight at deck height.

:::caution[A homing shot that loses its target detonates at the map corner]
A steered projectile loses its target when that target is destroyed, captured, enters a transport or a structure, or starts to cloak. With nothing to home on, the projectile stops moving and counts as arrived once its speed is 2 leptons a frame or more. Arriving moves it onto its homing point, which is now the coordinate `0,0,0`, the corner of the cell grid outside the playfield. The blast, its explosion and its lighting all happen there, and nothing near the flight path is damaged. An [`Airburst=yes`](/keys/airburst/) projectile, or one already at ground level, goes off where it stands instead.
:::

## Ballistic flight

Each frame, a ballistic projectile's vertical speed drops by [`[AudioVisual] Gravity`](/keys/gravity/), or by half of it for a [`Floater=yes`](/keys/floater/) projectile. The projectile then moves by its whole velocity.

Several tests below ignore friendly objects: those that belong to the firer's house or to a house allied with it.

A projectile forgets its firer when the firer is destroyed, captured, enters a transport or a structure, or starts to cloak. For the rest of its flight nothing counts as friendly, so the shot can go off on its own side's units and structures.

The projectile has landed when a step takes it below ground level, through a bridge deck, or into an obstacle. It is then placed on the ground or deck it struck.

An obstacle is a structure or a wall in the cell, and it counts only while the projectile is less than 150 leptons above the ground. These structures do not stop the shot:

- the structure that fired it;
- a [slack laser fence segment](/systems/laser-fences/#what-a-live-run-stops);
- a structure that counts as a deployed vehicle;
- a friendly structure.

Walls have no exemptions, so a wall also stops ballistic fire from its owner's house and allies. Only two overlays count as walls in this test: the ones at positions 2 and 26 of the `[OverlayTypes]` list, counting from 0. The stock list puts `GAWALL` and `NAWALL` there.

Another overlay marked [`High=yes`](/keys/high/#scope-overlaytype) stops a shot only through the tall-overlay test in [What ends a flight](#what-ends-a-flight). That test applies to both flight models and exempts a projectile marked [`High=yes`](/keys/high/#scope-bullettype). A `High=yes` projectile gets no exemption from the wall test above. The stock sandbag wall, `GASAND`, sets [`Wall=yes`](/keys/wall/) but not `High=yes`, so neither test stops a shot passing over it.

Every landing works out a rebound, even when the projectile is about to detonate. The velocity is scaled by [`Elasticity`](/keys/elasticity/#scope-bullettype) and reflected off the slope of the ground it landed on, so a shot that lands on a ramp bounces off downhill. What happens next depends on [`Bouncy`](/keys/bouncy/):

- A projectile that is not `Bouncy=yes` detonates where it landed.
- A `Bouncy=yes` projectile detonates when the cell it was in at the start of the turn holds an object that is not friendly. The firer's cell never counts. Otherwise it flies on.
- Any projectile detonates on its third landing.

Three more checks can end a ballistic flight after the move:

1. A projectile not already marked to detonate goes off on an object that is not friendly when that object stands in the cell the projectile just entered, within 128 leptons (half a cell) of it.
2. A projectile that has left the playfield is removed at its previous position without detonating, even if it had just landed.
3. A projectile moving slower than 10 leptons a frame, and lying within 10 leptons of the ground, detonates where it lies.

## What ends a flight

This table gathers every way a flight ends, in the order a turn tests them. The middle column gives the flight model each applies to. The last column says where the blast is placed, since several of these move the projectile first.

| What ends the flight | Applies to | Where it goes off |
| --- | --- | --- |
| The projectile is [`Dropping=yes`](/keys/dropping/) | Both | Where it stands, one frame's travel from where it was launched |
| It has arrived: half a frame's travel or less to go, or it is at ground level | Steered | On its homing point, unless it is [`Airburst=yes`](/keys/airburst/) or at ground level |
| It has stopped gaining on its target | Steered | Where it stands. `Airburst=yes` and [`VeryHigh=yes`](/keys/veryhigh/) are exempt |
| It crossed the height of a bridge deck | Steered | At deck height |
| It landed, and either is not [`Bouncy=yes`](/keys/bouncy/) or the cell it was in at the start of the turn holds an object that is not friendly | Ballistic | On the ground or deck it struck |
| It landed for the third time | Ballistic | Where it came down |
| It came within 128 leptons of an object that is not friendly, in the cell it just entered | Ballistic | On that object |
| It left the playfield | Ballistic | Nowhere. It is removed without a blast |
| It is moving slower than 10 leptons a frame and lies within 10 leptons of the ground | Ballistic | Where it lies |
| A [`Ranged=yes`](/keys/ranged/) projectile has used up its allowance | Both | Where it stands |
| It stands in a raised [firestorm wall](/systems/laser-fences/#projectiles) that does not let it through | Both | Nowhere. The wall consumes it without a blast |
| A [`High=yes`](/keys/high/#scope-overlaytype) overlay stands in its cell, the projectile is less than 100 leptons above the ground, and it is not [`High=yes`](/keys/high/#scope-bullettype) itself | Both | Where it stands |
| It is below ground level | Both | Where it stands |
| It is [`AA=yes`](/keys/aa/) and within 128 leptons of the aircraft, or the airborne jumpjet infantry, it is chasing | Both | Where it stands |
| Its fuse tripped | Steered, but never a `Dropping=yes` projectile | Where it stands. A trip within 64 leptons of the fuse point moves it onto the target, as described below |

## Where the blast lands

The blast does not always go off where the projectile stood. Up to three moves pull it onto the target, applied in this order. None of them applies once the projectile has lost its target.

1. The blast moves onto the target's center when the projectile is not [`Airburst=yes`](/keys/airburst/) and **Any of:**
   - the fuse tripped within 64 leptons of the point it was set on;
   - a ballistic shot landed and went off because it is not `Bouncy=yes`, or because the cell it was in at the start of the turn held an object that is not friendly. It must also have come down within 384 leptons (a cell and a half) of the target, or within six times its rebound speed if that is farther. Only half the height difference counts toward that distance.
2. The blast moves onto the target's center when it is within 32 leptons of it. `Airburst=yes` and [`Inaccurate=yes`](/keys/inaccurate/) projectiles skip this move.
3. Unless the warhead is [`EMEffect=yes`](/keys/emeffect/) or the projectile is [`Splits=yes`](/keys/splits/), the blast moves onto a nearby target. An airborne target within 128 leptons of the projectile pulls the blast onto the point weapons aim at on it. Any other target within 42 leptons pulls the blast onto its position, and a structure's distance is measured to its edge. `Inaccurate=yes` does not prevent this move.

The warhead decides what the blast does. [`AnimList`](/keys/animlist/) chooses the explosion by damage, [`SplashList`](/keys/splashlist/) replaces it over water, and [`Spread`](/keys/spread/#scope-warheadtype) sets how the damage thins with distance. An [`EMEffect=yes`](/keys/emeffect/) warhead spends the damage as [a pulse](/systems/emp-pulse/) instead.

## Clusters and splitting

[`Cluster`](/keys/cluster/) counts one of two things, and [`Splits`](/keys/splits/) chooses which. An `Airburst=yes` projectile splits unless it sets `Splits=no`; the `Splits` page explains why.

An ordinary projectile repeats its whole detonation `Cluster` times. The first blast lands on the point of impact. Each later blast lands one to two cells from that same point, in a random direction, so the blasts do not walk away from it.

A splitting projectile detonates once and then releases `Cluster` bomblets of its [`AirburstWeapon`](/keys/airburstweapon/). The split runs in this order, which decides what a bomblet can still target:

1. The carrier's blast, explosion and lighting flash happen.
2. The candidate list is built from the objects within five cells of the carrier's target, or of the carrier itself when its target is a cell. Objects the carrier's blast destroyed are already gone.
3. Each bomblet keeps the carrier's target at the [`RetargetAccuracy`](/keys/retargetaccuracy/) chance, and otherwise draws from the list.
4. The bomblets are created at the carrier's position, pointing straight down. That position ignores the 32-lepton move onto the target.

## Where a projectile's artwork is read from

Write `Image=` in a projectile's rules section, even when it would name the section itself. Repeat it in every rules file that declares the section, including a map's rules. A section without `Image=` clears the projectile's image name.

Every object type reads [`Image=`](/keys/image/) with its current image name as the default, and that name starts as the section name. A projectile then reads `Image=` a second time, with an empty default. When the section does not set `Image=`, the second read leaves the projectile with no image name.

The art settings read after that point are affected:

- The shape artwork survives, because it was already loaded under the earlier name and a failed load keeps it.
- [`Trailer`](/keys/trailer/) and [`Rotates`](/keys/rotates/) are not read at all.
- [`AnimLow`](/keys/animlow/), [`AnimHigh`](/keys/animhigh/), [`AnimRate`](/keys/animrate/) and [`AnimPalette`](/keys/animpalette/) are looked up in an art section with an empty name, so they are not found.

These six settings keep their defaults, or the values an earlier file gave them. An art section named after the projectile does not supply them either. Each of their key pages records the effect on that setting.

:::danger[Give every voxel projectile an `Image=` in every rules file]
The voxel model is loaded after the image name has been cleared, so the load fails, and a failed voxel load discards any model the projectile already had. The projectile stays [`Voxel=yes`](/keys/voxel/) with no model behind it. The game crashes the first time it draws such a projectile, which happens once the projectile is on screen, not under fog, and not [`Inviso=yes`](/keys/inviso/).
:::
