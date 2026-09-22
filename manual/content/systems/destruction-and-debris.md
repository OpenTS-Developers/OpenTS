---
title: Destruction and debris
summary: "What happens when a vehicle, structure, infantry soldier or aircraft is destroyed, and what it leaves on the ground."
category: weapons-projectiles
keys:
  - C4Warhead
  - CollateralDamageCoefficient
  - Crater
  - CrewEscape
  - Crewed
  - Cyborg
  - DeadBodies
  - DeathFrames
  - DebrisMaximums
  - DebrisTypes
  - Doggie
  - Explodes
  - Explosion
  - FirestormWarhead
  - Immune
  - InfDeath
  - InfantryExplode
  - Insignificant
  - LargeFire
  - MaxDebris
  - MaxDeathCounter
  - MetallicDebris
  - ScrapExplosion
  - ScrapMetal
  - Scorch
  - SpawnsTiberium
  - SmallFire
  - SplashList
  - Storage
  - SurvivorDivisor
  - TiberiumExplosive
  - TiberiumHeal
  - Wake
related:
  - type: system
    id: projectile-flight
  - type: system
    id: capture
  - type: system
    id: tiberium
  - type: system
    id: veterancy
  - type: command
    id: CenterOnRadarEvent
---

A destroyed vehicle, structure, infantry soldier or aircraft goes through two steps. The shared step runs first for every kind of object: it throws wreckage and sets off the collateral blast. The step for the object's kind follows. It plays the death animation, lets any survivors walk away, and takes the object off the map. Three cases change that sequence: a fall into water, a vehicle's wreck animation, and a structure whose removal is delayed.

This page starts once the object is destroyed. [Projectile flight and impact](/systems/projectile-flight/) covers where a blast lands and how many blasts a shot delivers. [Engineers, capture and sabotage](/systems/capture/#survivors) owns the number of soldiers a destroyed structure releases, because selling a structure uses the same count. That count also decides whether the structure's [footprint walk](#survivors-and-the-scarring-are-one-walk) runs.

## Writing the settings

Write the per-type settings in the object type's rules section. The wreck animation's frame counts go in the art section that the type's `Image=` names. The shared animations and warheads, such as `SmallFire` and `C4Warhead`, are global rules settings; each key page names its section. The values below are examples.

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
Explodes=yes           ; sets off the collateral blast
Crewed=yes             ; lets one crew member escape
MaxDebris=5            ; at most five pieces of wreckage
DebrisTypes=TIRE,WTRAK ; VoxelAnimTypes registered in [VoxelAnims], thrown in order
DebrisMaximums=4,2     ; at most four TIRE, then at most two WTRAK
```

```ini title="art.ini"
[MYTANK] ; the art section the type's Image= names
DeathFrames=2      ; frames in the wreck animation
MaxDeathCounter=16 ; game frames the wreck stands before it explodes
```

## The step every kind shares

The shared step runs first, in this order, for a vehicle, a structure, an infantry soldier and an aircraft alike.

1. One of the type's [`VoiceDie`](/keys/voicedie/) sounds plays at the object's position.
2. The object breaks radio contact, stops, and drops its target and destination.
3. A [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) type seeds [Tiberium](/systems/tiberium/) in the five cells to its north-west, north, east, south and west. Each cell that accepts growth gets a random stage from 0 to 2 of the first registered Tiberium type. The object's cell gets none.
4. Any flame particle system attached to the object is removed.
5. **The water exit.** The rest of this step is skipped when **all of** these hold:
   - the object is no more than 10 [leptons](/glossary/#lepton) above the ground;
   - it fell from a height, for example when a bridge collapsed under it;
   - the ground beneath it is water.

   The infantry step, and the step for a vehicle without [`DeathFrames`](/keys/deathframes/), then leave a wake and a splash in place of the usual death animation.
6. **The wreckage.** An object whose type sets [`MaxDebris`](/keys/maxdebris/) above zero throws wreckage. A type with a [`DebrisTypes`](/keys/debristypes/) list throws those animations from its center. A type without one throws [`MetallicDebris`](/keys/metallicdebris/) animations from twenty leptons above its center. `MaxDebris` caps the number of pieces either way.
7. **The collateral blast.** An [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) type, or an object whose rank grants [the explodes ability](/systems/veterancy/#abilities), deals area damage sized by [`CollateralDamageCoefficient`](/keys/collateraldamagecoefficient/). The blast plays a combat explosion animation and uses the warhead of the object's current primary weapon: its elite weapon at elite rank, or an upgrade's weapon on a structure. A [`Bright=yes`](/keys/bright/#scope-warheadtype) warhead adds a lighting flash. An object with no weapon in that slot gets no animation, flash or damage from this step. `Explodes` owns the radius and damage figures.

Because the water exit comes before the wreckage and the blast, a vehicle without `DeathFrames` that falls off a bridge into a river leaves only the splash, whatever its wreckage and explosion settings. A vehicle with `DeathFrames` also skips the wreckage and the blast, but it still becomes [a wreck](#a-vehicle). The wreck gets no splash, and it plays its death explosion when its counter runs out.

### Spilled harvester loads

An object carrying Tiberium, such as a harvester, spills its load only when it is `Explodes=yes` or holds the explodes ability. It spills even when it has no weapon and so sets off no collateral blast. A harvester that is neither keeps its load and is taken off the map with it. A scenario with [`HarvesterImmune=yes`](/keys/harvesterimmune/) spills nothing. A structure's stored Tiberium is spilled in [the structure's step](#a-structure) instead.

The spill is always the first registered Tiberium type, whatever the harvester carried. The number of placements is nine times the fraction of its [`Storage`](/keys/storage/) that it held, rounded down, but at least one. Each placement adds a random stage from 0 to 2 to one neighboring cell. The cells come in a fixed order that starts at the north-west cell and returns to it on the third placement. A full load therefore reaches all eight neighbors, and a smaller load reaches fewer.

The load can also explode. That blast is a separate setting, [`TiberiumExplosive`](/keys/tiberiumexplosive/#scope-global-rules). It goes off when the vehicle plays its death explosion: at [an outright death](#a-vehicle) that is neither a firestorm kill nor a fall into water, and when its wreck finally explodes. A vehicle with no death explosion, from [`Explosion`](/keys/explosion/) or from `ScrapExplosion` under `ScrapMetal`, never sets it off.

## The loss announcement

When a vehicle, soldier or aircraft owned by the player at this machine is destroyed, EVA announces the loss. The cell that [Goto Radar Event](/commands/centeronradarevent/) jumps to moves to where the object was heading, which is the first of these that applies:

1. the exit of the tunnel it is traveling through;
2. the coordinate its [locomotor](/glossary/#locomotor) was driving toward;
3. its center, when it was going nowhere.

Three deaths are not announced, and they leave the Goto Radar Event cell where it was:

- a type marked [`Insignificant=yes`](/keys/insignificant/);
- a structure, whatever its settings;
- a vehicle whose artwork declares [`DeathFrames`](/keys/deathframes/), including when its wreck finally explodes.

## A vehicle

A vehicle whose artwork declares [`DeathFrames`](/keys/deathframes/) survives the destroying hit as a wreck. It is put back to one point of strength and stays where it is, unable to move, while it plays its wreck animation. When its counter passes [`MaxDeathCounter`](/keys/maxdeathcounter/), it plays its death explosion, picked the same way as for an outright death below, and it is taken off the map. The wreck path releases no passengers, no crew and no truck crate. [`CrewEscape`](/keys/crewescape/) covers the crew roll that it skips.

:::caution[Hits on a wreck count as new kills]
Nothing records that a wreck has already died. It stays on the map and holds its cell, and any hit that takes its last point of strength destroys it again and puts it back to one point. Each such hit:

- books the kill again: score and any experience for the attacker, and another entry in both houses' loss and kill counts;
- runs the whole shared step again, so the wreck throws a fresh batch of wreckage, and an `Explodes=yes` vehicle sets off a fresh collateral blast;
- offers the destruction events to the wreck's tag again. [Destroyed by any house](/mapping/events/tevent-destroyed/) needs an attacker, and [Destroyed by anything](/mapping/events/tevent-destroyed-any/) does not. [Trigger persistence](/systems/trigger-springing/) decides whether a tag is still there to take them.
:::

A vehicle without `DeathFrames` is finished outright. Its death animation is the first of these that applies:

1. Killed by the [`[CombatDamage] FirestormWarhead`](/keys/firestormwarhead/): seven to nine firestorm particle systems.
2. Fallen into water, as the water exit describes: a [`Wake`](/keys/wake/) and the last entry of [`SplashList`](/keys/splashlist/).
3. Otherwise: one random entry of its [`Explosion`](/keys/explosion/) list, where it stood. With [`ScrapMetal`](/keys/scrapmetal/) on, a type that names a [`ScrapExplosion`](/keys/scrapexplosion/) list uses that list instead. An `Explodes=yes` vehicle, or one with the explodes ability, plays the last entry of the list while it still has [ammunition](/keys/ammo/), which includes unlimited ammunition.

The rest follows in this order:

1. A train car is uncoupled, and the cars behind it stop.
2. The vehicle is lifted off its cells.
3. Each passenger gets out if it can enter the vehicle's cell, and the rest are killed. Every passenger is killed when the damage was forced or the vehicle fell from a height.
4. One crew member may escape; [`CrewEscape`](/keys/crewescape/) owns that roll. A vehicle taken by a hijacker skips the roll, and [the hijacker steps back out](/systems/capture/#stealing-a-vehicle) instead.
5. A crate-carrying truck may drop a wooden crate on a nearby cell, when the scenario enables [truck crates](/keys/truckcrate/) or, for a train, [train crates](/keys/traincrate/).
6. The vehicle is taken off the map.

## A structure

A structure's step runs in the order below. Its *origin cell* is the cell at the top corner of its footprint.

1. A unit in radio contact with it, such as a harvester docked at a refinery or an aircraft on its pad, is destroyed when its center is less than one cell from the structure's center. No attacker is credited for that kill. A unit farther away is told to move off.
2. The structure's light source is switched off.
3. Everything inside it is killed. The effects it supplied end: vision from a spied radar structure, a cloak generator's field, and a laser fence post's connections.
4. **The central ground mark.** A structure at least two cells wide *and* two cells deep lays one mark on its origin cell. A smaller structure lays none. The mark is a **smudge**, a flat stain on the ground that stays once laid. It is a scorch or a crater, with even odds, and it lands only where a smudge type of that kind fits; [`Scorch`](/keys/scorch/) lists what makes a spot fit. The structure standing on the cell does not block this mark, but Tiberium or another overlay there does. A multiple-cell smudge type is preferred, and a single-cell type is used when no larger one fits.
5. **Fire and explosions over the footprint.** Each footprint cell has an even chance of a [`SmallFire`](/keys/smallfire/), and a cell that gets one has an even chance of a [`LargeFire`](/keys/largefire/) beside it. Each cell also plays one entry of the type's [`Explosion`](/keys/explosion/) list, or its `ScrapExplosion` list with `ScrapMetal` on, drawn separately for each cell.
6. An [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) structure places a `FIRE3` fire animation on any [explosive overlay](/keys/explodes/#scope-overlaytype) in the four cells that share an edge with its origin cell. On a structure at least two cells wide and deep, two of those cells lie inside its footprint. The explodes ability does not count here.
7. Whatever the structure was storing is spilled one unit at a time. Each unit keeps its Tiberium type and lands one to three cells from the origin cell, wherever the ground accepts growth; [credits and storage](/systems/tiberium/#credits-and-storage) covers the rest.
8. Strength is set to zero. Forced damage, which skips the armor table and [`Immune=yes`](/keys/immune/), marks the structure as leaving no survivors. The footprint walk below then runs.

### Survivors and the scarring are one walk

The footprint walk releases survivors and marks the ground in one pass over the footprint. It takes the survivor count first, and a count of zero ends the walk before it starts, so the footprint gets neither survivors nor marks from it. The count is zero when **any of** these holds:

- the structure is not [`Crewed=yes`](/keys/crewed/);
- it was destroyed by forced damage;
- the game uses a [`SurvivorDivisor`](/keys/survivordivisor/) of `0`.

When the count is above zero, each footprint cell in turn may release one survivor, at the odds [capture](/systems/capture/#survivors) sets out, until the count runs out. A multiplayer house whose player has resigned gets no survivors, and neither does one whose player left a game without AI takeover.

The same cell then takes a mark: a scorch or a crater, with even odds. Only a cell a tracked vehicle could enter takes a mark, and infantry, vehicles and structures on the cell do not count against it. A cell whose land type has a [`Track=`](/systems/movement-and-terrain/#the-terrain-table) cost of zero takes no mark, so a structure standing partly on water or rock is marked in patches.

A mark cannot land on a cell a structure still stands on, and the walk runs while the destroyed structure is still in place. An ordinary destruction therefore leaves no marks from the walk. Only the second walk of a [delayed removal](#when-the-structure-leaves-the-map) marks the footprint.

### When the structure leaves the map

An ordinary structure is taken off the map on the frame it dies, right after the walk. Two kinds stay at zero strength and are removed on their next update:

- a structure that was being sold when it died;
- an `Explodes=yes` structure. The explodes ability does not delay removal.

The delayed removal lifts the structure off its cells, runs the footprint walk a second time, and then deletes the structure.

:::caution[A delayed removal releases survivors twice]
Nothing records that the walk has already run. A structure removed late therefore releases up to twice the usual number of soldiers: once when it dies and once when it is removed.
:::

## Infantry and aircraft

A forced kill of a [`Cyborg=yes`](/keys/cyborg/) soldier always removes it, whatever death is chosen below, and leaves an [`InfantryExplode`](/keys/infantryexplode/) animation when the soldier fell from a height. A soldier's death is the first of these that applies:

1. the water exit from the shared step, which here leaves a [`Wake`](/keys/wake/) and the first entry of [`SplashList`](/keys/splashlist/);
2. a prone `Cyborg=yes` soldier, which leaves `InfantryExplode` (a standing cyborg [survives its first unforced killing hit](/keys/cyborg/) and goes prone);
3. a jumpjet soldier, which leaves `InfantryExplode`;
4. otherwise the death that the killing warhead's [`InfDeath`](/keys/infdeath/) names. A kill by a [laser fence](/systems/laser-fences/) uses the electrocution death, or the burning death on a [`Doggie=yes`](/keys/doggie/) type.

The soldier stays on the map to play three of those deaths: the gun death, the explosion death, and the burning death of a `Doggie=yes` type. When a gun or explosion death finishes, a soldier that is not `Doggie=yes` leaves a corpse from [`DeadBodies`](/keys/deadbodies/). Every other death removes the soldier at once and leaves an animation in its place, or nothing.

An aircraft plays one entry of its [`Explosion`](/keys/explosion/) list where it was hit, or of its `ScrapExplosion` list with `ScrapMetal` on. A kill by the firestorm warhead plays seven to nine firestorm particle systems instead. An aircraft on the ground is then removed at once.

An aircraft in the air falls instead. Its passengers are killed, and it falls faster and faster until it reaches the ground. There it explodes with a fixed 1000 points of area damage through [`C4Warhead`](/keys/c4warhead/), credited to no one. The crash plays a combat explosion animation sized to that figure, with a lighting flash when `C4Warhead` is `Bright=yes`, and the aircraft is removed. Its wreckage and its `Explosion` entry appear where it was hit, not where it crashes.

## What the ground keeps

A vehicle, soldier or aircraft lays no ground mark itself. A structure lays [its central mark](#a-structure), and its footprint walk lays more on a delayed removal. Every other mark comes from an animation the death created that declares [`Scorch=yes`](/keys/scorch/) or [`Crater=yes`](/keys/crater/#scope-animtype).

Such an animation stamps the ground on its largest frame if it is under 30 leptons above the ground. The mark is picked at random each time from the smudge types with the matching flag that fit the spot; [`Scorch`](/keys/scorch/) lists what makes a spot fit. No mark lands on a cell a structure still stands on, except a structure's central mark. A [`Flamer=yes`](/keys/flamer/) or `Scorch=yes` animation also starts fires, within the height and ground limits that [`Scorch`](/keys/scorch/#the-fire) gives.

The table lists what can remain after each kind of object dies, besides the Tiberium a [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) type seeds. Check it when a death leaves less than you expect.

| Left behind | Vehicle | Structure | Infantry | Aircraft |
| --- | --- | --- | --- | --- |
| Wreckage from [`DebrisTypes`](/keys/debristypes/) or [`MetallicDebris`](/keys/metallicdebris/) | Where `MaxDebris` is above 0 | Where `MaxDebris` is above 0 | Where `MaxDebris` is above 0 | Where `MaxDebris` is above 0, at the point of the kill |
| A mark from a death animation | Where the animation sets `Scorch` or `Crater` | Where the animation sets `Scorch` or `Crater` | Where the animation sets `Scorch` or `Crater` | Where the animation sets `Scorch` or `Crater` |
| Marks laid by the object itself | No | The central mark on a structure at least 2×2, and footprint marks on a delayed removal | No | No |
| Fire animations | Only from a `Flamer=yes` or `Scorch=yes` animation | Up to two per footprint cell | Only from a `Flamer=yes` or `Scorch=yes` animation | Only from a `Flamer=yes` or `Scorch=yes` animation |
| A corpse | No | No | After a gun or explosion death, except on a `Doggie=yes` type | No |
| Soldiers who walk away | Passengers who can get out, and one crew member on the crew roll or the hijacker who stole it; none from a wreck | Up to five, twice that on a delayed removal | No | No |
| Tiberium | A harvester's load, when it is `Explodes=yes` or has the explodes ability | Whatever it was storing | No | No |

A terrain object, such as a tree, leaves nothing standing. When one is destroyed, it starts to crumble and is removed from the map in the same step, so the crumble is never seen. A [`SpawnsTiberium=yes`](/keys/spawnstiberium/) tree skips the crumble. It explodes with 100 points of damage through `C4Warhead` and can set off [the Tiberium in its cell](/systems/tiberium/#damage).
