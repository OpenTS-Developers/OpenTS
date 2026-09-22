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

Both barriers make a line of cells lethal. When a barrier goes live, it destroys every vehicle, aircraft and infantryman standing in its cells. Afterwards it turns back or destroys whatever tries to cross.

The two differ in what switches them on. Each laser fence run is switched by the two posts at its ends, so one base can have some runs live and others **slack** at the same time. A slack run's segments stay on the map but stop nothing. A house's firestorm wall sections are all switched together by one superweapon. For a house a human is playing, keeping the wall up spends that weapon's charge.

## Laser fences

A [`LaserFencePost=yes`](/keys/laserfencepost/) structure is built and paid for like any other. The [`LaserFence=yes`](/keys/laserfence/) segments between posts are free: a post creates them, switches them and removes them. When a scenario is read, every post lays its runs again, so a map needs to list only the posts.

```ini title="rules.ini"
[NAPOST]          ; the post, registered in [BuildingTypes]
LaserFencePost=yes
GuardRange=10     ; how far the post searches for a partner

[NAFNCE]          ; the segment type; only one LaserFence=yes type is ever laid
LaserFence=yes
```

### Finding the far post

A post searches north, east, south and west for a partner, up to [`GuardRange`](/keys/guardrange/) cells in each direction. The range is cut down to whole cells, and a value below one cell counts as one, so a post always reaches its four neighboring cells. The same `GuardRange` also sets how far the post looks for targets while guarding.

In each direction the search stops at the first building. That building becomes the partner only if **all of** these hold:

- it is a laser fence post;
- it belongs to the same house;
- it is not being sold or undeployed.

Any other building in the way, including another house's post, leaves that direction without a run.

### Laying the run

Every post lays the same `LaserFence=yes` type, so define only one. Any other `LaserFence=yes` type is never laid.

Segments are created for the post's house and placed one cell at a time toward the partner. A segment cannot be placed where a structure could not be built: on a bridge, a ramp, a cell where a bridge once stood, a wall or other overlay that blocks building, or ground that does not allow building. Tiberium and veins are the exception, so a run can cross a field. A segment can share its cell with vehicles and infantry, but not with a building or a terrain object such as a tree.

If any segment cannot be placed, the whole run in that direction is removed, so a run is either complete or absent.

Laying a run also burns the ground under it, from the laying post's cell to the last segment. Each cell with Tiberium loses twelve stages of it; [Tiberium](/systems/tiberium/#damage) covers what that costs a field. Veins that belong to a [veinhole monster](/systems/veins/) are cut back, and veins that no monster owns are not reduced. Because the burn includes the post's cell, it happens even between two adjacent posts.

### Energizing the run

A run is live only while both of its posts meet **all of** these:

- it is [operational](/systems/power/#defenses), which rules out being switched off, stunned by an [EM pulse](/systems/emp-pulse/), or short of power;
- it has finished its buildup;
- it is not being sold or undeployed.

If either post fails a test, the whole run goes slack. No segment in between is removed.

When a run goes live, every vehicle, aircraft and infantryman standing in one of its cells is destroyed. A vehicle, aircraft or infantryman that later enters a live cell is destroyed the same way. Each victim takes damage equal to its current strength through [`C4Warhead`](/keys/c4warhead/), and the segment counts as the attacker. The damage is forced, so neither armor nor [`Immune=yes`](/keys/immune/) reduces it. An infantryman killed by a segment dies as with [`InfDeath=5`](/keys/infdeath/), whatever the warhead sets.

### What a live run stops

A live and a slack segment treat each of these as follows:

| Subject | Live segment | Slack segment |
| --- | --- | --- |
| Vehicles and infantry | Cannot enter the cell | Pass through |
| Infantry standing in the cell on Guard | Scatter away | Stay put |
| Projectiles that are not homing ([`ROT=0`](/keys/rot/#scope-bullettype)) | Strike the segment, unless the firer is allied to its house | Pass through |
| Particles and bouncing debris | Strike the segment | Pass through |

A live segment stops a projectile, particle or piece of debris only while it passes through the cell less than 150 [leptons](/glossary/#lepton) above the ground. A shell arcing higher crosses over the fence.

A projectile that strikes a live segment detonates on it. A [`Bouncy=yes`](/keys/bouncy/) projectile bounces off instead. It still detonates if it lands in a cell holding an object that is not allied to its firer.

Segments take only forced damage, so ordinary weapon fire never destroys one. A segment is removed only when its post removes the run, or when a post or gate is placed on it.

:::caution[Path planning does not see a laser fence switch]
Switching a run on or off does not update the passability data that path planning uses. Path planning keeps what it last recorded for those cells, which for a newly laid run is open ground. Vehicles and infantry are still turned back, because each one refuses a live cell when it tries to enter. The firestorm wall does update that data, so path planning routes around a raised wall.
:::

### What lowers or removes a run

These events make a run slack and leave its segments in place:

- a post's house runs short of power, if the post is one that stops working in a shortage (see [Power](/systems/power/#defenses));
- a post is switched off;
- a post is [stunned by an EM pulse](/systems/emp-pulse/#what-a-pulse-reaches);
- a post starts being sold or undeployed.

The run goes live again when both posts are operational once more. Every recalculation of a house's power balance rechecks all of that house's posts.

Removing a post removes the segments of every run it holds:

- **Destroyed:** each segment is destroyed by forced damage equal to its strength through `C4Warhead`, with no attacker. Each one flashes if that warhead sets [`Bright=yes`](/keys/bright/#scope-warheadtype).
- **Sold, undeployed or otherwise taken off the map:** the segments are deleted.
- **Captured:** the segments are deleted for the old owner. The post then searches all four directions again and lays new runs for its new owner.

Placing a structure on a fence also cuts it. Only a laser fence post or a [`Gate=yes`](/keys/gate/) structure can be placed on a segment, and only on a segment of the same house:

- A post placed on a segment deletes only that segment.
- A gate placed over segments follows the fence to a post, which then removes that whole run. If the fence leads to no post, only the segment under the gate is deleted.

## The firestorm wall

A [`FirestormWall=yes`](/keys/firestormwall/) BuildingType is an ordinary structure on the map. Sections are placed one at a time, each on a cell that holds no other object. Each placement also fills the gap to a nearby section of the same house.

```ini title="rules.ini"
[GAFSDF]                        ; the wall section
FirestormWall=yes
GuardRange=5                    ; how far a placed section fills a gap

[GAFIRE]                        ; the generator that grants the weapon
SuperWeapon=FirestormSpecial

[FirestormSpecial]              ; the superweapon that switches the wall
Type=Firestorm
UseChargeDrain=true
RechargeTime=3

[General]
GDIFirestormGenerator=GAFIRE    ; the one type that keeps a raised wall up
ChargeToDrainRatio=.333
DamageToFirestormDamageCoefficient=.1
```

These are the values the shipped `rules.ini` uses.

### Filling the gap

When a section is placed, the engine scans north, east, south and west from it for up to the section type's `GuardRange` cells, cut down to whole cells. Unlike a fence post there is no one-cell minimum, so a `GuardRange` below one cell fills nothing. The shipped wall sets `GuardRange=5`.

In each direction, the gap is filled with free sections when **all of** these hold:

- the scan finds another section of the same house within range;
- every cell between the two passes the ordinary placement test.

The scan stops at the first cell that fails the placement test, and nothing is filled in that direction. [Production](/systems/production/#leaving-the-factory) covers the placement step that starts the fill.

### Raising and lowering the wall

Raising the wall switches every section the house owns at once. It also updates the passability data for those cells, so path planning treats the raised wall as impassable. Lowering reverses both. The local player hears an announcement when their wall goes down. A lowered section blocks nothing and is crossed like open ground.

Each section's frame shows how it connects to the sections beside it, so corners, tees and straight runs are drawn differently. The frame number adds 1 for a section to the north, 2 for east, 4 for south and 8 for west, plus 32 while the wall is up. A section's art therefore holds its raised frames 32 after its lowered ones.

While the wall is up, sections that are not part of a straight run show [`FirestormActiveAnim`](/keys/firestormactiveanim/), and the key page covers when a section loses it. Every eighth frame, each such section also has a one-in-sixteen chance to start [`FirestormIdleAnim`](/keys/firestormidleanim/). A straight north-south or east-west section shows neither.

### What a raised section destroys

A raised section runs two sweeps whenever it updates: when its house raises the wall, and when the section itself, or a section beside it, is placed or removed while the wall is up. The sweeps use different warheads and catch different objects:

- **Its own cell:** every vehicle, aircraft and infantryman standing there takes damage equal to its current strength through [`FirestormWarhead`](/keys/firestormwarhead/), with no attacker. [`FirestormAirAnim`](/keys/firestormairanim/) appears at the victim when it is more than 100 leptons above the ground; otherwise [`FirestormGroundAnim`](/keys/firestormgroundanim/) appears at the section.
- **The approach:** the sweep checks the 5×5 block of cells centered on the section, excluding the section's cell. Every vehicle or infantryman there that is moving into the section's cell takes damage equal to its current strength through `C4Warhead`, with no attacker and no animation.

The approach sweep catches only objects whose [locomotor](/keys/locomotor/) is drive, hover, mech or walk. The other six locomotors never report moving into the cell: flying, jumpjet, tunnel, teleport, levitate and ballistic.

Movement over a raised wall is checked continuously. An object with the flying or jumpjet locomotor that moves over a raised section's cell takes the same `FirestormWarhead` damage and animation as the cell sweep. Ground movers never reach that point, because a raised section's cell is closed to them.

None of these checks spares the wall's own house or its allies. A type that declares [`IgnoresFirestorm=yes`](/keys/ignoresfirestorm/) is skipped by the cell sweep, the approach sweep and the flying-object check.

:::caution[`IgnoresFirestorm=yes` does not protect a jumpjet]
The jumpjet check does not read `IgnoresFirestorm`. A jumpjet that declares it is still destroyed when it moves over a raised section.
:::

An aircraft killed by `FirestormWarhead` does not play its usual explosion. It spawns seven to nine [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) particle systems, each throwing sparks in random directions. A vehicle killed by that warhead does the same unless its art sets [`DeathFrames`](/keys/deathframes/).

### Projectiles

A projectile that reaches a raised section's cell is destroyed without detonating. The section shows its air or ground animation, chosen by the same height test as the cell sweep. An [`Inviso=yes`](/keys/inviso/) projectile has no visible flight, so it is instead checked along the line from where it was fired to its target. It is destroyed at the first raised section on that line.

Two exemptions let a projectile through:

- it was fired by an object of the wall's own house;
- its type declares `IgnoresFirestorm=yes`.

The `Inviso=yes` line check applies only the first exemption, so an `IgnoresFirestorm=yes` type is still destroyed on that path. The exemption covers the wall's own house only, not its allies. A projectile with no firer is always destroyed.

### Damage while the wall is up

A raised section never takes damage. Instead, the incoming damage, before armor, is multiplied by [`DamageToFirestormDamageCoefficient`](/keys/damagetofirestormdamagecoefficient/) and taken off the countdown of the house's first `Type=Firestorm` superweapon, never below zero. For a house a human is playing, firing on the wall therefore shortens how long it stays up. It does not open a hole.

Every section takes damage like any other structure while the wall is down.

### Selling a section

A wall section whose art has no [`Buildup`](/keys/buildup/) animation, like the shipped one, is sold differently from other structures. A player can sell it only while its house's wall is down, and the sale removes it at once.

:::caution[Selling a wall section returns nothing]
A section sold this way refunds nothing, whether the house paid for it or the gap fill placed it for free.
:::

## The firestorm generator and its charge

A [`Type=Firestorm`](/keys/type/#scope-superweapontype) superweapon raises the wall. With [`UseChargeDrain=yes`](/keys/usechargedrain/), as shipped, the same weapon also lowers it and spends its charge while the wall is up. Without that flag the wall, once raised, never comes down.

A house receives the weapon from a structure whose type names it in [`SuperWeapon=`](/keys/superweapon/). [Superweapons](/systems/superweapons/) covers the rest of the weapon's lifecycle.

### Charge and drain

The weapon keeps one countdown for both charging and draining. Charging starts it at [`RechargeTime`](/keys/rechargetime/), which is written in minutes. When it reaches zero, the weapon is ready and can raise the wall. Raising and lowering the wall convert the countdown between charge and drain time.

In the table, `remaining` is the countdown at the moment of the switch, in the same unit as `RechargeTime`. [`ChargeToDrainRatio`](/keys/chargetodrainratio/) is one value in `[General]`, shared by every charge-draining weapon.

| Event | New countdown |
| --- | --- |
| The wall is raised from ready | `(RechargeTime − remaining) × ChargeToDrainRatio` |
| The wall is lowered by hand | `RechargeTime − (remaining ÷ ChargeToDrainRatio)` |
| The countdown runs out while the wall is up | a full `RechargeTime`, and the wall lowers |

The first two rows are inverses, so the charge can be spent in parts. A wall raised from a full charge stays up for `RechargeTime × ChargeToDrainRatio`. If it is lowered early, the countdown left is the time it was up divided by `ChargeToDrainRatio`. A ratio below `1` therefore costs more recharge time than the wall was up, and a ratio above `1` costs less.

A wall lowered by hand can be raised again at once. The weapon keeps the countdown from the table, and raising the wall converts it back into drain time.

A power shortage that suspends the weapon also lowers the wall and discards all of its charge. [Power](/systems/power/#superweapons) covers when a shortage suspends it.

### Losing the generator

Only the BuildingType named by [`GDIFirestormGenerator`](/keys/gdifirestormgenerator/) keeps a raised wall up. The `SuperWeapon=` assignment does not count. When a structure of that type is switched off or taken off the map while the wall is up, the engine looks for another one that meets **all of** these:

- its type is the one `GDIFirestormGenerator` names;
- it belongs to the same house;
- it is switched on;
- it is out of [limbo](/glossary/#limbo);
- it is not being sold or undeployed;
- it has finished its buildup.

If none qualifies, the wall comes down as if it had been lowered by hand.

### Computer houses

A computer house never raises its wall on its own. Its superweapon AI handles other superweapon types but not this one. A computer house's wall goes up only through the [Activate Firestorm Defense](/mapping/actions/taction-activate-firestorm/) trigger action. The [Deactivate Firestorm Defense](/mapping/actions/taction-deactivate-firestorm/) action is one of several things that lower it.

Both actions work through the house's copy of the first `Type=Firestorm` superweapon the rules declare, whether or not any structure grants it to the house. Each does nothing when the wall is already in the state it asks for. When that weapon sets `UseChargeDrain=yes`, a computer house's wall switches at once, even if the house has no generator. For a house a human is playing, they act like using the weapon by hand: raising the wall works only while the weapon is ready.

:::caution[A computer house's wall never drains]
The charge-and-drain formulas apply only to a house a human is playing. In a campaign, that is only the player's house, even when a trigger gives another house a generator. For every other house, raising or lowering the wall leaves the countdown alone, and a charged weapon's countdown stays at zero. Such a wall stays up until a trigger, a power loss or the loss of the generator lowers it. Firing on it never brings it down.
:::
