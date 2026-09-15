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

A projectile has none of a shot's own figures. The firing weapon hands over the damage, the warhead, the launch speed, the flight allowance and the object credited with a kill. The projectile's own type settles how the shot travels, what ends the journey, and how many blasts it leaves behind.

Two flight models exist and one setting chooses between them. A projectile whose [`ROT`](/keys/rot/#scope-bullettype) is above zero is flown by the missile autopilot, which turns it toward its target each frame and lifts it over the ground in the way. A projectile with no rate of turn is left to the arc it was launched along. Gravity pulls it down, and it bounces off whatever it lands on. Almost every setting below belongs to one model or the other, and a setting written on the wrong kind of projectile changes nothing at all.

[Firing geometry and beam weapons](/systems/firing-geometry/) owns where a shot appears and what is drawn alongside it. This page owns the shot itself, from the velocity it leaves with to the last blast it delivers.

## Projectile types in brief

A **BulletType** is a rules section describing one kind of shot. No list registers it. A weapon's [`Projectile=`](/keys/projectile/) names it, and a name the game has not met before is created on the spot. A misspelling therefore produces a projectile with none of the intended settings rather than an error.

The division of labor between the weapon section and the projectile section is fixed. The weapon holds how hard the shot hits, how far it may be fired and how often. The projectile holds how it flies, what it may be aimed at, and what its impact is broken into.

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

At runtime a projectile is an object that stands in no cell. It claims no cells of its own and is never entered into a cell's list of occupiers, so nothing runs into it and nothing that scans a cell finds it. It is submitted to the air layer whatever its height.

## What the shot leaves with

Five things are settled at the moment of launch, and the projectile then lives with all of them.

The aim point is solved to where the firer expects its current target to have got to, not to where that target stands. A projectile that is both [`Inaccurate=yes`](/keys/inaccurate/) and [`Arcing=yes`](/keys/arcing/) has that point displaced first, by [`BallisticScatter`](/keys/ballisticscatter/). Everything below is then solved to the displaced point.

The direction comes from the firer's own gun heading on a homing or [`Dropping=yes`](/keys/dropping/) projectile, whatever direction the target lies in. Every other projectile leaves pointed at the aim point. The gun heading is the turret facing on a vehicle or aircraft that has a turret, and the body facing on one that does not and on infantry. On a building it is the turret facing, or the direction from the building's center to what it is shooting at where there is no turret.

The pitch is decided by the first of these that applies:

- an [`Arcing=yes`](/keys/arcing/) projectile has a ballistic arc solved for it, high or flat, which the weapon's [`Lobber`](/keys/lobber/) setting picks between, and the shot is abandoned outright when no arc at that speed reaches;
- a [`Voxel=yes`](/keys/voxel/) projectile leaves straight up instead of level;
- otherwise the shot leaves level unless the aim point lies more than 200 leptons above or below the mounting, in which case it is pitched at that point with 20 leptons taken off the height difference first. A lepton is the engine's internal distance unit, at 256 to the cell, and a terrain level is 104 of them, so 200 leptons come to a little under two terrain levels. A structure is the exception: the target's height is taken as 200 times its [`Height`](/keys/height/#scope-buildingtype) before the same 20 leptons come off.

The launch speed is the weapon's [`Speed=`](/keys/speed/#scope-weapontype), which for any unguided projectile the rules pass has already replaced with a figure derived from the weapon's [`Range=`](/keys/range/). It is then held to half the distance to the aim point, so a shot taken at very close quarters leaves more slowly than the same shot taken across the weapon's full reach. On an arcing shot the arc is solved at that reduced speed. A homing projectile ignores both figures at launch: it starts at one lepton a frame and works up to the weapon's figure over the course of the flight, as [steered flight](#steered-flight) describes.

Only a homing projectile ever reads a fuse. It is armed with the aim point and the type's [`Arm`](/keys/arm/) delay, which that page covers along with the two ways it trips.

An [`Inviso=yes`](/keys/inviso/) projectile skips the flight entirely. It is placed on its target's own coordinate as it is launched, not on the aim point solved above.

:::caution[An aircraft overwrites the launch velocity it was just given]
When the firer is an aircraft, the shot's velocity is rewritten after it has been created and launched. Which rewrite it gets depends on the exact rate of turn.

| `ROT` | What the aircraft imposes |
| --- | --- |
| `0` | Level pitch, the aircraft's own turret heading, and the aircraft's current travel speed. The arc solved above, the pitch and the launch speed are all discarded, so an [`Arcing=yes`](/keys/arcing/) bomb dropped from the air does not follow the arc its weapon worked out |
| `1` | Aimed straight at the target's center in three dimensions, at the full [`Speed=`](/keys/speed/#scope-weapontype) of the weapon in the aircraft's first weapon slot, whichever weapon slot the shot came from, rather than along the gun heading at one lepton a frame |
| `2` or more | Nothing. The launch stands as solved |

A rate of turn of exactly `1` is therefore not simply a very slow missile when the firer is an aircraft: it is a missile that starts pointed at its target and at full speed.
:::

## The turn

Projectiles take their turns at a fixed point in the logic frame, after animations and building lights. Structures, aircraft, infantry and vehicles all take theirs afterwards. Each list is walked from the end toward the start.

A projectile therefore moves before anything it might hit has moved on that frame, and it reads its target where the previous frame left it.

The steps below run in this order on every turn.

1. A [`Dropping=yes`](/keys/dropping/) projectile is marked for detonation.
2. The flight animation advances one step when either [`AnimLow`](/keys/animlow/) or [`AnimHigh`](/keys/animhigh/) is other than zero.
3. A [`Trailer`](/keys/trailer/) puff is dropped at the position the projectile is about to leave, on every third frame of the game clock.
4. One of the two flight models steps the projectile forward and may mark it for detonation.
5. A [`Ranged=yes`](/keys/ranged/) projectile pays the distance it just covered out of its allowance and is marked when the allowance runs out.
6. The new position is taken up. A projectile standing in a raised [firestorm wall](/systems/laser-fences/#projectiles) is consumed there and the turn ends.
7. A projectile not already marked is put through the three standing tests listed below: a tall overlay in its cell, ground level, and the anti-aircraft proximity.
8. A homing projectile checks its fuse.
9. Either the projectile is still flying, in which case a [`Degenerates=yes`](/keys/degenerates/) projectile loses a point of damage, or it detonates and is removed.

A shot from a weapon mounted on a barrel is the exception: two of these turns run for it immediately, inside the firing sequence, so that it appears past the muzzle rather than inside it. [The firing sequence](/systems/firing-geometry/#the-shot-step-by-step) covers which shots are given that head start. Every other shot waits until the next logic frame for its first step.

## Steered flight

A projectile with a rate of turn is flown by the autopilot every frame. Its speed is worked toward the weapon's figure, gaining [`Acceleration`](/keys/acceleration/#scope-bullettype) a frame. A projectile that is moving faster than that figure coasts back down at half that rate. A projectile is still launching until it has reached cruising speed or been given a weapon [`Speed=`](/keys/speed/#scope-weapontype) of `16` or more, which is `40` leptons a frame on that key's 0-to-100 scale. While it is still launching, the gain is one lepton every other frame instead, and it does not turn at all. The rate of turn itself is scaled up and down again on a fifteen-frame cycle by [`MissileROTVar`](/keys/missilerotvar/), which is what makes a missile weave. Over the last cell of the approach it is increased by half again.

Within one frame the autopilot takes one of two branches. It follows the terrain under **all of:**

- the target is not an aircraft;
- **Any of:** the projectile is [`Airburst=yes`](/keys/airburst/); the horizontal distance still to run is over three cells, or over six for a [`VeryHigh=yes`](/keys/veryhigh/) projectile;
- the frame's turn allowance is more than one direction unit, a 256th of a turn.

A projectile that is still launching is given no turn allowance at all, and at the engine's own [`MissileROTVar`](/keys/missilerotvar/) a projectile whose [`ROT`](/keys/rot/#scope-bullettype) is `1` never rises above one either. Neither of those ever follows the terrain. A `ROT` of `2` or more does clear the gate at that value of `MissileROTVar`.

Terrain following samples the ground six frames of travel ahead and holds the projectile a fixed clearance above it, nudging its height 18 leptons at a time. It turns its pitch up, down or level by half the frame's turn allowance. Otherwise the projectile dives: its pitch is turned straight at the target with an allowance so large that it effectively snaps. The clearance the following branch holds, and the distance at which the dive begins, are what [`VeryHigh`](/keys/veryhigh/) and [`Airburst`](/keys/airburst/) change.

The projectile has arrived once the distance still to run is no more than half of one frame's travel, or once it has reached the ground. That distance counts only a quarter of the height difference, and none of it at all for an airburst projectile. A projectile that arrives while still in the air is moved onto the aim point first, unless it is an airburst.

A projectile that is not gaining on its target is made to detonate rather than circle it forever. How fast it is closing is accumulated plainly over its first sixty frames of cruise, then kept as a decaying total. The flight ends once that total lands anywhere between zero and sixty leptons. A projectile that is losing ground has a negative average and is not caught by it. The test therefore reaches the one that has stopped closing, not the one that is running away. `Airburst=yes` and `VeryHigh=yes` projectiles are exempt, since both are meant to sit above their targets rather than close on them.

Crossing the plane of a bridge deck, from above or from below, ends the flight at deck height.

:::caution[A homing shot whose target is taken off the map detonates at the corner of the world]
A projectile is detached from its target when that target is destroyed, enters a transport or a building, or merely cloaks. The detach ignores the flag that separates a permanent loss from a temporary one. The projectile is left with no target and no aim point, and the autopilot answers a missing aim point by holding the projectile still and reporting one lepton left to run. That is inside half a frame's travel for anything moving faster than two leptons a frame, so the projectile counts as having arrived. The arrival step then moves it onto its aim point, which is now the coordinate `0,0,0`, the corner of the cell array outside the playfield. The blast, its animation and its lighting all happen there, and nothing near the flight path is touched. An [`Airburst=yes`](/keys/airburst/) projectile is spared the move and goes off where it stands.
:::

## Ballistic flight

A projectile with no rate of turn is stepped along its arc. Its vertical speed loses [`[AudioVisual] Gravity`](/keys/gravity/) each frame, halved for a [`Floater=yes`](/keys/floater/) projectile, and the whole velocity is then added to its position.

Where that step puts the projectile at or below the ground, or through a bridge deck, or into an obstacle, the projectile has landed. The obstacle test looks for a structure or a wall standing in the cell, and only while the projectile is within 150 leptons above the ground. Four kinds of structure are passed through untouched: the firer's own, a [slack laser fence segment](/systems/laser-fences/#what-a-live-run-stops), one that counts as a deployed vehicle, and one belonging to an ally of the firer. A wall is given none of those exemptions, so a house's own wall stops its own ballistic fire exactly as an enemy's does. Only two entries in the overlay list are recognized as walls by this test, the brick wall and the Nod wall, `GAWALL` and `NAWALL`. An overlay marked [`High=yes`](/keys/high/#scope-bullettype) that is neither of them stops a shot through [the standing test](#what-ends-a-flight) later in the same turn instead, which reaches both flight models and exempts only a `High=yes` projectile. A sandbag wall sets [`Wall=yes`](/keys/wall/) but no `High=yes`, so neither test stops a shot through it.

A landing always computes a rebound, whether or not the projectile will survive it. The velocity is rotated into the frame of reference of the ground slope, scaled by [`Elasticity`](/keys/elasticity/#scope-bullettype), then reflected and rotated back out. That is what throws a shot that lands on a ramp off downhill. [`Bouncy`](/keys/bouncy/) decides whether the projectile then carries on or is detonated where it came down.

Two further things can end a ballistic flight in mid-air. A projectile that passes within 128 leptons (half a cell) of an object that is neither the firer nor one of the firer's allies detonates on that object. A projectile that leaves the playfield is removed at its last known position without detonating at all.

Two dead branches sit in this step and are worth naming so that nobody tunes against them. A test at the head of the step marks a projectile moving slower than eight leptons a frame as having landed. It sets only the kind of the impact and never the flag that ends the flight. Since the impact kind on its own is treated the same as no impact, nothing follows from the test. The test at the foot of the step is the one that ends the flight. It applies to a projectile moving slower than ten leptons a frame and lying within ten leptons of the ground. A projectile aimed at a cell rather than at an object was meant to detonate as soon as it landed in that cell. The cell-target test sits after the one that has already forced every projectile which is not `Bouncy=yes` to detonate, so it is never reached.

## What ends a flight

Every route to a detonation is gathered here, in the order a single turn tests them. The middle column is the flight model each applies to, and the last says where the blast is placed, since several of them move the projectile before it goes off.

| What ends the flight | Applies to | Where it goes off |
| --- | --- | --- |
| The projectile is [`Dropping=yes`](/keys/dropping/) | Both | Where it stands, which is one frame's travel from the muzzle |
| It has arrived: no more than half a frame's travel left, or it reached the ground | Steered | On the aim point, unless it is [`Airburst=yes`](/keys/airburst/) or already on the ground |
| It has stopped gaining on its target | Steered | Where it stands. `Airburst=yes` and [`VeryHigh=yes`](/keys/veryhigh/) are exempt |
| It crossed the plane of a bridge deck | Steered | At deck height |
| It landed, and either is not [`Bouncy=yes`](/keys/bouncy/) or bounced out of a cell holding an object that is not the firer or an ally | Ballistic | On the surface it struck |
| It has bounced three times | Ballistic | Where it came down |
| It passed within 128 leptons of an object that is not the firer or an ally | Ballistic | On that object |
| It left the playfield | Ballistic | Nowhere — it is removed without a blast |
| It is moving under ten leptons a frame and lying within ten of the ground | Ballistic | Where it lies |
| A [`Ranged=yes`](/keys/ranged/) projectile has spent its allowance | Both | Where it stands |
| It is standing in a raised [firestorm wall](/systems/laser-fences/#projectiles) | Both | Nowhere — it is consumed without a blast |
| A tall overlay stands in its cell, it is below 100 leptons, and it is not [`High=yes`](/keys/high/#scope-bullettype) | Both | Where it stands |
| It is below ground level | Both | Where it stands |
| It is [`AA=yes`](/keys/aa/) and within 128 leptons of the aircraft or airborne jumpjet it is chasing | Both | Where it stands |
| Its fuse tripped | Steered, and never a `Dropping=yes` projectile | Where it stands, before the adjustments below |

## Where the blast lands

The point of impact is not simply where the projectile was standing. Up to two adjustments move it, and they run in this order.

First, a projectile that went off within 32 leptons of its target's center is moved onto that center, so that the explosion is seen to strike the victim. `Airburst=yes` and [`Inaccurate=yes`](/keys/inaccurate/) both suppress this one. The airburst is left where it goes off so that it stays overhead, and a shot thrown off its aim point on purpose is not pulled back onto the target.

Then, provided the warhead is not [`EMEffect=yes`](/keys/emeffect/) and the projectile is not [`Splits=yes`](/keys/splits/), one further nudge is applied. Where the target is airborne and the projectile went off within 128 leptons of it, the blast moves to the target's own aim point. Otherwise, where it went off within 42 leptons of any target, the blast moves onto that target's coordinate.

A third adjustment is written alongside those two and cannot be reached. It would pull the blast back onto the point the fuse was aimed at. It asks for a projectile that neither homes nor arcs and that was not forced to detonate, but a projectile that does not home never reads its fuse, so being forced is the only way it ever detonates.

What the blast then does belongs to the warhead: [`AnimList`](/keys/animlist/) chooses the explosion by damage, [`SplashList`](/keys/splashlist/) replaces it over water, and [`Spread`](/keys/spread/#scope-warheadtype) sets how the damage thins with distance. An [`EMEffect=yes`](/keys/emeffect/) warhead spends the damage as [a pulse](/systems/emp-pulse/) instead.

## Clusters and splitting

[`Cluster`](/keys/cluster/) counts one of two very different things, and [`Splits`](/keys/splits/) chooses which. An ordinary projectile repeats its whole detonation that many times. The first blast lands on the point of impact, and every later one is thrown one to two cells from it. Each is drawn afresh from that same original point, so the blasts do not walk away from it. A splitting projectile detonates exactly once and then releases that many bomblets of its [`AirburstWeapon`](/keys/airburstweapon/).

The ordering inside a split is worth stating because it decides what a bomblet can still find. The carrier's own blast, its explosion animation and its lighting flash all happen first. The candidate list is then gathered from the objects standing within five cells of the carrier's target, or of the carrier itself when the target is a cell. The bomblets are created last, at the carrier's own position rather than at the adjusted point of impact. Anything the carrier's blast destroyed is therefore already gone from the list, and [`RetargetAccuracy`](/keys/retargetaccuracy/) governs the draw against what remains.

## Where a projectile's artwork is read from

Write an `Image=` in every projectile's section. A projectile is the one object type whose image name can be cleared by the file that describes it. Every type reads [`Image=`](/keys/image/) with its current name as the default, so leaving the key out keeps whatever the name already was. That name starts as the section's own name. A projectile then reads the same key a second time with an empty default, into the same storage. A section that does not set `Image=` is left holding no image name at all.

The shape artwork survives that, because it was already fetched under the section name by the first read and a failed fetch does not clear what is there. What does not survive is every art section lookup that follows. [`Trailer`](/keys/trailer/) and [`Rotates`](/keys/rotates/) are read only when that second read still found a name, so they are skipped outright. [`AnimLow`](/keys/animlow/), [`AnimHigh`](/keys/animhigh/), [`AnimRate`](/keys/animrate/) and [`AnimPalette`](/keys/animpalette/) are looked up under an empty section name and keep whatever they held. Those four key pages each record the effect on their own setting. This is the mechanism behind all of them, and the reason a section named after the projectile is not read either.

:::danger[A voxel projectile that omits `Image=` has no model left to draw]
The voxel model is fetched after the name has been cleared rather than before, so the fetch fails, and a failed voxel fetch discards the whole set rather than leaving the previous one in place. The projectile is then left marked [`Voxel=yes`](/keys/voxel/) with nothing behind it. The drawing routine reads the model's layer count without first testing that a model is there, so the game stops the first time such a projectile is drawn. That needs the projectile to be on screen, not fogged, and not [`Inviso=yes`](/keys/inviso/). Give every voxel projectile an `Image=` in its rules section.
:::
