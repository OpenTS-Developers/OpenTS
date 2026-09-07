---
title: Destruction and debris
summary: "Works a destroying hit through one shared step and then the object's own, and settles what is left standing on the cell."
category: weapons-projectiles
keys:
  - C4Warhead
  - CollateralDamageCoefficient
  - Crater
  - CrewEscape
  - Crewed
  - DeadBodies
  - DeathFrames
  - DebrisMaximums
  - DebrisTypes
  - Explodes
  - Explosion
  - FirestormWarhead
  - InfDeath
  - InfantryExplode
  - LargeFire
  - MaxDebris
  - MaxDeathCounter
  - MetallicDebris
  - ScrapExplosion
  - ScrapMetal
  - Scorch
  - SmallFire
  - SplashList
  - Storage
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
---

A destroying hit is worked through twice. One step runs for every kind of object and settles the wreckage and the collateral blast; a second belongs to the kind — vehicle, structure, infantry or aircraft — and settles the explosion animation, who walks away from it, and when the object is taken off the map. Which of the two steps a setting is read at decides whether the sequence's three exits — a fall into water, a wreck animation, a structure's delayed removal — reach it at all.

What put the damage there is not this page's concern. [Projectile flight and impact](/systems/projectile-flight/) owns where a blast is placed and how many of them a shot delivers. The soldiers a destroyed structure turns loose belong with the rest of its crew handling in [engineers, capture and sabotage](/systems/capture/#survivors), because that count is read on a sale as readily as on a destruction and is not a step of this sequence at all. One consequence of the count is a step of it, and [the structure section](#survivors-and-the-scarring-are-one-walk) covers that.

## The step every kind shares

The shared step runs first, in this order, on a vehicle, a structure, an infantryman and an aircraft alike.

1. One entry of the type's death voice is played at the object's position.
2. Radio contact is broken and the object is stunned.
3. A [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) type spreads [Tiberium](/systems/tiberium/) into the five cells north-west, north, east, south and west of it. Its own cell takes none.
4. Any flame particle system attached to it is deleted.
5. **The water exit.** Everything below is skipped under **all of:** the object stands no more than 10 leptons above the ground; it was knocked off a height and is falling to its destruction; the land type beneath it is water. No debris and no blast follow, and the kind's own step substitutes a wake and a splash.
6. **The wreckage.** [`MaxDebris`](/keys/maxdebris/) above zero throws pieces from [`DebrisTypes`](/keys/debristypes/) at the object's center, or from [`MetallicDebris`](/keys/metallicdebris/) twenty leptons above it where the type declares no list of its own.
7. **The collateral blast**, on an [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) type or one whose crew has earned [the explodes ability](/systems/veterancy/#abilities): a combat explosion animation, a lighting flash and area damage sized by [`CollateralDamageCoefficient`](/keys/collateraldamagecoefficient/), carrying the warhead of the object's own first weapon.

Two readings off that order are worth keeping. Debris is thrown before the blast, so a type that both sheds wreckage and explodes scatters its pieces into its own explosion rather than out of it. And the water exit sits above both, which is why a vehicle knocked off a bridge into a river leaves nothing but the splash, whatever its wreckage settings say.

A destroyed harvester's cargo is spilled inside step 7 rather than beside it. The Tiberium goes out one cell at a time to the eight neighbors, reaching as many of them as the fraction of [`Storage`](/keys/storage/) it was carrying covers, and always as the first registered Tiberium type whatever it had aboard. Because the spill sits inside the collateral blast, a harvester that is neither `Explodes=yes` nor carrying the ability keeps its load and is taken off the map with it. The extra blast the load itself produces is a separate setting, [`TiberiumExplosive`](/keys/tiberiumexplosive/#scope-global-rules).

## A vehicle

A vehicle whose artwork declares [`DeathFrames`](/keys/deathframes/) is not finished by the hit. It is put back to one point of strength, left standing where it is, and plays a wreck animation until its counter passes [`MaxDeathCounter`](/keys/maxdeathcounter/); only then is the explosion animation created and the vehicle taken off the map. Nothing else in this section is reached on that path — no passengers thrown clear, no crew, no crate — and [`CrewEscape`](/keys/crewescape/) covers the survivor that is passed over.

:::danger[A wreck playing a death animation can be killed over and over]
The wreck is still on the map, still holding its cell, and nothing records that it has already died. Every further hit that takes its one point off drives it to zero again, books the kill again — score for the attacker, experience toward the attacker's next rank, and another entry in both houses' loss and kill tallies — runs the whole shared step again, and puts the wreck back to one point. A vehicle standing in sustained fire therefore sheds a fresh batch of [`DebrisTypes`](/keys/debristypes/) wreckage, and an `Explodes=yes` one a fresh collateral blast, for every hit it absorbs while the animation plays. The destroyed trigger event is spared this, because a vehicle never springs it.
:::

A vehicle finished outright takes one of three exits, and the first that applies wins:

- killed by [`[CombatDamage] FirestormWarhead`](/keys/firestormwarhead/), it is replaced by seven to nine firestorm particle systems;
- falling to its destruction over water, it leaves a [`Wake`](/keys/wake/) and the last entry of [`SplashList`](/keys/splashlist/);
- otherwise one entry of its [`Explosion`](/keys/explosion/) list is created where it stood, or of its [`ScrapExplosion`](/keys/scrapexplosion/) list where the game is played with [`ScrapMetal`](/keys/scrapmetal/) on and the type names one.

The rest follows in order: a train car releases whatever was following it, the vehicle is lifted off its cells, passengers are put out or killed with it, the crew rolls to escape, and a crate-carrying truck may drop a wooden crate on a nearby cell.

## A structure

A structure's own step runs in this order.

1. A vehicle in radio contact with it — a harvester at a refinery, say — is destroyed outright when it stands within one cell of the structure's center, and told to get out of the way otherwise. Any light it was casting is switched off.
2. Everything loaded inside is killed with it, and the standing effects it supplied are unwound: leaked vision from a spied radar structure, a cloak generator's field, a laser fence post's connections.
3. **The central ground mark.** A structure at least two cells wide *and* two cells deep lays one mark at its own cell: an even chance of a scorch and otherwise a crater. A **smudge** is that mark — a flat stain laid on the ground, which stays there once laid. Only the multiple-cell smudge types are eligible here, and the structure's own footprint does not block the placement. A structure narrower or shallower than that lays nothing, and the mark is never offset inside the footprint.
4. **Fire and explosions over the footprint.** Each cell takes an even chance of a [`SmallFire`](/keys/smallfire/), and half of those a [`LargeFire`](/keys/largefire/) beside it; each cell separately takes one entry of the type's [`Explosion`](/keys/explosion/) list, drawn afresh per cell rather than once for the building.
5. An [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) structure sets fire to any [explosive overlay](/keys/explodes/#scope-overlaytype) standing in the four cells beside it.
6. Whatever the structure was storing is spilled onto the surrounding cells as [Tiberium](/systems/tiberium/), one unit at a time.
7. Strength is fixed at zero, forced damage — the kind that skips the armor table and [`Immune=yes`](/keys/immune/) — marks the structure survivorless, and the walk below runs.

### Survivors and the scarring are one walk

The soldiers and the ground marks come out of the same walk over the footprint, and the survivor count is taken before the walk begins. **A count of zero abandons the walk outright.** A structure that is not [`Crewed=yes`](/keys/crewed/), one destroyed by forced damage, and one playing under a [`SurvivorDivisor`](/keys/survivordivisor/) of `0` all leave their footprint completely unmarked — no scorch and no crater on any cell of it. Only the single central mark from step 3 survives, and only on a structure large enough to have laid one.

Where the count is above zero, each footprint cell in turn offers one survivor at the odds [capture](/systems/capture/#survivors) sets out, and then takes a mark of its own on the same even chance between a scorch and a crater. A cell only takes that mark if a tracked vehicle could stand there — infantry and any building or vehicle on the cell are disregarded, but one whose land type prices [`Track=`](/systems/movement-and-terrain/#the-terrain-table) at exactly zero takes nothing, which is why a structure standing partly on water or on rock comes out patchily marked.

### When the structure leaves the map

An ordinary structure is removed on the same frame it dies, immediately after that walk. A structure that was already deconstructing when it died, and any `Explodes=yes` structure, is not: it is left standing at zero strength and removed on the next pass of its own logic instead.

:::danger[A structure removed late produces two sets of survivors and two sets of marks]
The delayed removal runs the survivor-and-scarring walk a second time before deleting the structure, and nothing records that it has already run. An `Explodes=yes` structure, and one destroyed while a sale was in progress, therefore turn out up to twice the intended number of soldiers and mark their footprint twice over. The ordinary case escapes it only because it is deleted before its own logic can come round again.
:::

## Infantry and aircraft

A forced kill of a [`Cyborg=yes`](/keys/cyborg/) soldier is settled first and separately: the soldier is removed whatever follows, and one that was falling at the time leaves an [`InfantryExplode`](/keys/infantryexplode/) animation. The chain then runs regardless, and the first of these that applies wins:

1. the same water exit the shared step tested, which here leaves a [`Wake`](/keys/wake/) and the first entry of [`SplashList`](/keys/splashlist/);
2. a prone `Cyborg=yes` soldier, which leaves `InfantryExplode`;
3. a jumpjet soldier, which leaves the same;
4. otherwise the death the killing warhead's [`InfDeath`](/keys/infdeath/) names, forced to the electrocution death when a [laser fence](/systems/laser-fences/) did the killing.

Only three of those deaths are played out on the body itself — the gun death, the explosion death, and a [`Doggie=yes`](/keys/doggie/) type's burning death — and only the first two leave a corpse from [`DeadBodies`](/keys/deadbodies/), created when the sequence reaches its end rather than when the soldier dies. Every other route removes the soldier at once and leaves an animation in its place, or nothing.

An aircraft creates its [`Explosion`](/keys/explosion/) entry — or firestorm particle systems, on the firestorm warhead — and is then removed at once if it was on the ground. One in the air is not. It is stunned, its passengers are killed, and it falls, gathering speed, until it reaches the ground; there it goes off with a fixed 1000 points of area damage through [`C4Warhead`](/keys/c4warhead/) and nobody credited, with a combat explosion animation and a lighting flash sized to that figure, and is removed. Its debris was created at the shared step, where the aircraft was hit rather than where it came down, so a kill high above the ground scatters wreckage nowhere near the crash.

## What the ground keeps

The hit itself never lays a mark. Marks come from the animations the death created, and only from those declaring [`Scorch=yes`](/keys/scorch/) or [`Crater=yes`](/keys/crater/#scope-animtype), which stamp the ground as they reach their widest frame and only while standing under 30 leptons above it. Which mark is laid is chosen afresh every time, from every smudge type carrying the matching flag that will fit the spot.

The table gathers what can survive on the cell after each kind of object dies, beyond the Tiberium a [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) type of any kind spreads around it. It is what to check a wreck against when it leaves less than expected.

| Left behind | Vehicle | Structure | Infantry | Aircraft |
| --- | --- | --- | --- | --- |
| Voxel wreckage from [`DebrisTypes`](/keys/debristypes/) | Yes | Yes | Yes | Yes, at the point of the kill |
| A smudge from the explosion animation | Only where the animation carries the flag | Yes, and from the footprint walk besides | Only where the animation carries the flag | Only where the animation carries the flag |
| Fire animations | No | One or two per footprint cell | No | No |
| A corpse | No | No | Only on the gun and explosion deaths | No |
| Escaping soldiers | One, on the crew roll | Up to five, over the footprint | No | No |
| Tiberium | Only a loaded [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) harvester | Whatever it was storing | No | No |

A terrain object is the exception to all of it. One brought to zero strength starts its crumbling animation and is removed from the map on the same step, so the crumble is never seen; a [`SpawnsTiberium=yes`](/keys/spawnstiberium/) tree substitutes a 100-point blast and a chain reaction for the crumble instead. Either way the cell is left bare.
