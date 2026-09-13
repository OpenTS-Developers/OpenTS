---
title: Laser fences and the firestorm wall
summary: "Laser fence posts energize the segment runs between them, and one charge-draining superweapon raises every firestorm wall section a house owns."
category: buildings-economy
keys:
  - C4Warhead
  - ChargeToDrainRatio
  - DamageToFirestormDamageCoefficient
  - DefaultFirestormExplosionSystem
  - FirestormActiveAnim
  - FirestormAirAnim
  - FirestormGroundAnim
  - FirestormIdleAnim
  - FirestormWall
  - FirestormWarhead
  - GDIFirestormGenerator
  - GuardRange
  - IgnoresFirestorm
  - LaserFence
  - LaserFencePost
  - RechargeTime
  - SuperWeapon
  - Type
  - UseChargeDrain
related:
  - type: system
    id: power
  - type: system
    id: emp-pulse
  - type: system
    id: superweapons
  - type: action
    id: TACTION_ACTIVATE_FIRESTORM
  - type: action
    id: TACTION_DEACTIVATE_FIRESTORM
---

Both barriers turn a line of cells lethal: whatever is standing there as the barrier goes live is destroyed, and whatever tries to cross afterwards is turned back or destroyed in its turn. They differ in what decides that a cell is live. A laser fence run is switched by the two posts holding its ends, so one base can have some runs live and others **slack** — the segments still standing in their cells but carrying no charge and stopping nothing — at the same time. Every firestorm wall section a house owns is switched together by a single superweapon, and for a house a human is playing, keeping the wall up spends that weapon's charge.

## Laser fences

A [`LaserFencePost=yes`](/keys/laserfencepost/) structure is placed and paid for; the [`LaserFence=yes`](/keys/laserfence/) segments between posts are not. The post creates them, energizes them and tears them down, and they never appear in a scenario's own structure records — a post relays its runs from scratch each time a scenario finishes loading.

### Finding the far post

A post searches north, east, south and west, one cell at a time, for at most [`GuardRange`](/keys/guardrange/) cells. That value is written in cells; the fence truncates it to a whole number and treats anything below one cell as one, so a post always reaches its four neighboring cells however small the value is.

The search that lays a run stops at the first building of any kind, and accepts it as a partner only when it is another laser fence post of the same owner. That is what makes the run buildable: a partner found this way proves every cell strictly between the two posts is free of buildings. A foreign post, or any other structure standing in the way, ends the search with no partner and no run in that direction.

### Laying the run

The segment type is not named anywhere. The engine walks the declared BuildingTypes in order and uses the first one carrying `LaserFence=yes`, whichever post is asking, so a second `LaserFence=yes` type is never selected by this path.

Segments are created for the post's own house and placed one cell at a time toward the partner, all facing the axis of the run. A segment that cannot be placed is destroyed and the whole run in that direction is torn down before the placement returns, so a run is either complete or absent.

The engine then walks the same line a second time, starting on the post's own cell, and burns the ground under it: every cell carrying Tiberium loses twelve stages of it, and veins owned by a veinhole are cut back, while veins with no owner are only redrawn. Because the burn starts on the post's cell it always clears at least one cell, even when the two posts are adjacent. [Tiberium](/systems/tiberium/#damage) covers what losing those stages costs a field.

### Energizing the run

A run goes live only while both of the posts holding its ends clear **all of**:

- it is [operational](/systems/power/#defenses);
- it has not been powered down — a second flag, cleared by switching the structure off and by an [EM pulse](/systems/emp-pulse/), and restored only by switching it back on or by recovering from the stun;
- it is not on the construction mission;
- it is not on the deconstruction mission.

The two power terms are not one test. A pulse clears the second flag and re-evaluates the fence before it records the stun, so a stunned post's run drops on that flag rather than on the first term. Whatever takes one post out of service leaves the whole run slack; nothing in between is removed.

While a run energizes, every vehicle, aircraft or infantryman standing in one of its segments' cells takes its whole current strength as forced damage through [`C4Warhead`](/keys/c4warhead/), with the segment itself as the source, and dies. Forced damage skips the armor table and [`Immune=yes`](/keys/immune/) alike. An infantryman killed this way plays a fixed death animation, overriding the warhead's own [`InfDeath`](/keys/infdeath/). Arriving in a live segment's cell later has the same result: a vehicle, aircraft or infantryman that reaches such a cell dies on the same warhead with the same source.

### What a live run stops

The table sets the same segment in its two states against the four things that come up against it. Reading the two columns together is the point: the slack column has no row in which the segment does anything. A slack run is not a weaker barrier or a slower one — a mover, a shot and a bouncing shell all cross its cells exactly as they cross open ground — while the segments themselves are still drawn along the line, so a run whose post has gone dark still looks like a fence and stops nothing.

| Subject | Live segment | Slack segment |
| --- | --- | --- |
| Vehicles and infantry | Cannot enter the cell | Ignored; the cell is ordinary ground |
| Infantry already standing in it on Guard | Scatters away | Stays put |
| Projectiles | Treated as an obstacle | Pass through |
| Particles and bouncing debris | Treated as an obstacle | Pass through |

A segment also refuses every damage that is not forced, so ordinary weapon fire never removes one. What takes a segment off the map is its own post's teardown or a structure placed on top of it.

:::caution[Energizing a run leaves the map's movement data untouched]
Raising a laser fence run neither recalculates the passability recorded for its cells nor resets the movement zones, and the recalculation routine itself lets a live fence's blocked state be overwritten before it is stored. Vehicles and infantry are still turned back, because that block is decided by each mover as it looks at the cell, but path planning and zone connectivity go on treating the cells as open ground. The firestorm wall does both recalculations and does not have this problem.
:::

### What lowers or removes a run

Losing power, being switched off, being [stunned by an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches), starting construction and starting deconstruction all drop the run to slack and leave the segments in place; regaining power or recovering from the stun raises it again if the far post is also operational. Every recomputation of a house's power balance re-evaluates all of its posts.

Removing the post itself removes the segments:

- **Destroyed.** Each segment along every side that carried fence takes its whole current strength through `C4Warhead` with no source, which destroys it, and is preceded by a lighting flash sized from that same current figure when the warhead is [`Bright=yes`](/keys/bright/). Since a segment refuses every damage that is not forced, that figure is in practice the type's maximum.
- **Sold, undeployed or otherwise taken off the map.** The same segments are deleted silently.
- **Captured.** The segments are deleted silently for the old owner, and the new owner's post immediately re-runs all four searches and lays fresh runs of its own.

Two placements also cut a run. A gate put down on fence cells follows the fence back to its post, which then clears that whole run; a post put down on a fence cell deletes only the segment underneath it. A laser fence post and a gate are the only structures allowed onto a cell that already holds a laser fence segment of the same house. The segment itself may be placed onto Tiberium or veins, provided the cell clears **all of**:

- it is not a bridge;
- it was not under a bridge;
- it is not a ramp.

As a post clears its runs it records which of its four sides were carrying fence. No gameplay path reads that record back — only the multiplayer synchronization checksum folds it in — and a captured post rebuilds its runs by running the four searches again, not from what it recorded.

## The firestorm wall

A [`FirestormWall=yes`](/keys/firestormwall/) BuildingType stays a real structure on the map. Sections are placed one at a time on the ordinary building test — the cell must hold nothing at all — and each placement also fills the gap to a nearby section of the same house.

### Filling the gap

The fill scans north, east, south and west for at most `GuardRange` cells, truncated to whole cells, and unlike the fence post it has no one-cell floor: a type whose `GuardRange` resolves below one cell fills nothing at all. The stock firestorm wall carries `GuardRange=5`, so it reaches five cells.

A direction contributes sections only while **all of**:

- the scan finds another section of the same house in that direction;
- that section stands more than one cell away;
- every cell in between passed the placement test — a cell that refuses placement ends that direction there.

[Production](/systems/production/#leaving-the-factory) covers the hand-placement step that triggers the fill.

### Raising and lowering the wall

Raising is house-wide and immediate. Every section the house owns takes the raised shape at once, and the engine then resets the movement zones and recalculates all of the affected cells, so the raised wall is impassable to path planning as well as to individual movers. Lowering reverses both, and announces itself to a player-controlled house. A lowered section blocks nothing and is walked through like open ground.

A section's shape is read from its four cardinal neighbors, so a corner, a tee and a straight run are drawn differently. While the wall is up, a section that is not part of a straight run carries [`FirestormActiveAnim`](/keys/firestormactiveanim/) and, every eighth frame, has a one-in-sixteen chance of flickering [`FirestormIdleAnim`](/keys/firestormidleanim/) over itself; a straight run shows neither.

### What a raised section destroys

A section makes its two sweeps only as it refreshes its own state — when its house raises the defense, and when a section is placed or removed beside it while the defense is already up. They do not use the same warhead, and they do not reach the same kinds of object:

- **Its own cell.** Every vehicle, aircraft or infantryman standing there takes its whole current strength as forced damage through [`FirestormWarhead`](/keys/firestormwarhead/), with no source. [`FirestormAirAnim`](/keys/firestormairanim/) is created at the victim's position when the victim is more than 100 leptons above the ground, and [`FirestormGroundAnim`](/keys/firestormgroundanim/) at the section's own position otherwise.
- **The approach.** Every cell within two cells of the section in either direction, its own excluded, is checked for something whose movement is headed for the section's cell. Whatever is found takes its whole current strength as forced damage through `C4Warhead`, with no source and no animation at all.

The approach sweep reaches ground movement and nothing else. It asks each candidate's locomotor whether that locomotor is headed for the section's cell, and only four of [the ten](/keys/locomotor/) answer the question at all: drive, hover, mech and walk. Driven vehicles, hovercraft, walkers and infantry on foot are therefore its whole scope. Everything on the other six — flying, jumpjet, tunnel, teleport, levitate and ballistic — answers no and is never caught, and an aircraft or a jumpjet that is off the ground is not entered in any cell's occupancy for the sweep to find in the first place. That is what the `IgnoresFirestorm=yes` exemption on this sweep spares: those four ground kinds and no others.

Crossings are the part that runs continuously. A flying object passing over a raised section's cell, and a jumpjet moving onto one, take the cell sweep's payload as they arrive, on every step of their movement. A ground mover never reaches that point, because the cell is closed to it while the wall is up.

A type declaring [`IgnoresFirestorm=yes`](/keys/ignoresfirestorm/) is passed over by the cell sweep, the approach sweep and the flying-object crossing.

An aircraft killed by `FirestormWarhead`, and a vehicle killed by it whose art declares no death sequence of its own, skips the usual explosion and instead spawns between seven and nine [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) particle systems with randomized spark directions.

:::caution[A jumpjet is not exempted by `IgnoresFirestorm=yes`]
The crossing test for an ordinary flying locomotor checks the type's `IgnoresFirestorm=yes` before killing it. The jumpjet locomotor's own crossing test omits that check entirely, so a jumpjet declaring the flag is still destroyed the moment it moves onto a raised section's cell.
:::

### Projectiles

A projectile that reaches a raised section's cell is consumed rather than stopped: the section spawns its air or ground flare and the projectile is deleted where it stands, without its warhead ever detonating. A projectile whose type has no visible flight is instead tested along the whole line from firer to target and consumed at the first raised section it would cross.

Two exemptions let a projectile in flight through: it was fired by an object belonging to the wall's own house, or its type declares `IgnoresFirestorm=yes`. The line test used for a projectile with no visible flight applies only the first, so an `IgnoresFirestorm=yes` type is still consumed on that path. The first is decided per house, not per alliance, so an ally's fire is consumed exactly like an enemy's, and a projectile with no firer at all is consumed too.

### Damage while the wall is up

While its house owns a [`Type=Firestorm`](/keys/type/) superweapon, a raised section takes no damage: the raw incoming damage is instead multiplied by [`DamageToFirestormDamageCoefficient`](/keys/damagetofirestormdamagecoefficient/) and subtracted from the countdown of the first `Type=Firestorm` superweapon the house owns, floored at zero. Without such a superweapon the section is damaged like any other structure. For a house a human is playing, shooting the wall therefore shortens how long it stays up rather than opening a hole in it. Once the wall is down, sections take damage normally.

### Selling a section

A firestorm wall section has no buildup animation, so the ordinary sell path does not apply to it. It may be sold only while the house's defense is down, and the sale takes it off the map immediately.

:::caution[Selling a wall section returns nothing]
The sell path computes the section's price and discards the figure without crediting the house. Sections laid by the automatic gap fill cost nothing to place and refund nothing when removed, and a paid section refunds nothing either.
:::

## The firestorm generator and its charge

The wall is raised by a superweapon whose SuperWeaponType section carries `Type=Firestorm` and [`UseChargeDrain=yes`](/keys/usechargedrain/), granted to a house by a BuildingType's [`SuperWeapon=`](/keys/superweapon/) assignment. [Superweapons](/systems/superweapons/) covers the rest of the lifecycle; what follows is what the charge-drain cycle does here.

### Charge and drain

The weapon runs one countdown for both halves of the cycle. It is loaded with [`RechargeTime`](/keys/rechargetime/) and counts down to zero, at which point the weapon reports itself ready. Switching the wall on and off then converts between the two.

The table gives what each of the three transitions leaves in that single countdown. The first two run the same ratio in opposite directions, which is what makes the charge spendable in installments: raise a fully charged wall, hold it up for a spell, and drop it again, and the countdown left behind is that spell divided by `ChargeToDrainRatio`. A ratio below `1` therefore costs more recharge than the wall was up for, and one above `1` costs less.

| Transition | The countdown becomes |
| --- | --- |
| Switched on from ready | `(RechargeTime − remaining) × ChargeToDrainRatio` |
| Switched off while running | `RechargeTime − remaining ÷ ChargeToDrainRatio` |
| Runs out while the wall is up | a full `RechargeTime`, and the wall lowers |

[`ChargeToDrainRatio`](/keys/chargetodrainratio/) is one figure in `[General]` shared by every charge-draining weapon. Switching off returns the weapon to ready rather than to charging, so the wall can be switched straight back on with whatever charge the switch-off returned. Losing power suspends the weapon, which lowers the wall and costs it every second of charge it had — [power](/systems/power/#superweapons) covers that.

### Losing the generator

[`GDIFirestormGenerator`](/keys/gdifirestormgenerator/) names exactly one BuildingType, and that name — not the `SuperWeapon=` assignment — decides whether a raised wall stays up. Whenever such a structure is switched off or taken off the map while the wall is up, the house is searched for a replacement. A structure qualifies under **all of**:

- its type is the one `GDIFirestormGenerator` names;
- it belongs to this house;
- it is switched on;
- it is out of [limbo](/glossary/#limbo);
- it is not on the construction mission;
- it is not on the deconstruction mission.

If the search finds none, the firestorm superweapon is discharged and the wall comes down. A second BuildingType that also grants the superweapon does not keep the wall alive, and losing the named type brings the wall down even while that second type still stands.

### Computer houses

A computer house never raises the wall by itself. Its automatic superweapon handler has cases for the other superweapons and none for this one, so an AI wall goes up only through the [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) and [Deactivate Firestorm Defense](/mapping/actions/taction-deactivate-firestorm/) trigger actions, both of which do nothing when the wall is already in the state they ask for.

:::caution[A computer house's wall never drains]
The two formulas above run only for a house a human is playing. Every other house takes a branch that raises or lowers the wall without touching the countdown or the weapon's state, and such a weapon never enters the draining state at all: once its recharge reaches zero the countdown simply stays there. A wall raised by a trigger stays up until a trigger, a power failure or the loss of the generator lowers it, and damage converted into charge drain costs it nothing. In a campaign that branch covers every house except the player's, including one handed a generator by a trigger.
:::
