---
title: Tiberium
summary: "Tiberium grows and spreads across map cells, and harvesters carry it to refineries for credits."
category: buildings-economy
keys:
  - AllowTiberium
  - Buildable
  - ChainReaction
  - Color
  - Debris
  - Dock
  - DockUnload
  - Growth
  - GrowthPercentage
  - Harvester
  - HarvesterDumpRate
  - HarvesterLoadRate
  - HarvesterUnit
  - Image
  - Power
  - Refinery
  - SiloDamage
  - SpawnsTiberium
  - Spread
  - SpreadPercentage
  - Storage
  - Tiberium
  - TiberiumExplosionDamage
  - TiberiumExplosive
  - TiberiumFarScan
  - TiberiumGrows
  - TiberiumGrowthEnabled
  - TiberiumNearScan
  - TiberiumProof
  - TiberiumSpreads
  - TiberiumToSpawn
  - Value
related:
  - type: system
    id: produce-cash
  - type: action
    id: TACTION_TIB_GROWTH
  - type: enum
    id: LandType
---

## Tiberium types

```ini title="rules.ini"
[Tiberiums]
0=MyTiberium ; example type, registered in Tiberium slot 0

[MyTiberium]
Image=1
Value=25
Power=1
Color=NeonGreen
Growth=2200
GrowthPercentage=.09
Spread=2200
SpreadPercentage=.09
```

Each entry in `[Tiberiums]` names the rules section that defines one Tiberium type. Types are registered in the order of the entries, and each type's Tiberium slot is its position in that order, counted from 0. With the conventional `0=` through `3=` list, the entry numbers and the slots match.

An entry whose number is lower than the count of types registered so far does not register a new type. It re-reads the section of the type already in that slot, and the section name on the entry is ignored. Every other entry registers a new type in the next free slot, whatever its number.

If the named section does not exist, the type keeps its built-in values, which include no overlay set.

A type's slot decides which storage compartment its harvested Tiberium goes into, and how ripe its cells must be before they [spread](#spread). [`TiberiumToSpawn`](/keys/tiberiumtospawn/), Tiberium crates and several destruction effects also name a type by its slot.

:::danger[Register at most four types]
Harvesters, storage buildings and houses each hold exactly four Tiberium compartments, one for each slot from 0 to 3. A fifth type registers in slot 4. Harvesting, storing or spending it writes outside those compartments and corrupts memory.
:::

[`Image`](/keys/image/#scope-tiberium) selects the type's overlay set. Every set has twelve growth stages, and the set decides whether the type can grow on sloped ground.

:::note[The large-Tiberium set]
In the shipped rules, the twelve overlays of the [`Image=2`](/keys/image/#scope-tiberium) set have no shape of their own and show their [`CellAnim`](/keys/cellanim/) instead, so a cell looks the same at every stage. They also set `Land=Rock`, so harvesters cannot collect them. The set has no slope overlays, so a type using it never spreads onto a slope.
:::

:::danger[Give every registered type an overlay set]
Set [`Image`](/keys/image/#scope-tiberium) on every type, and give every entry in `[Tiberiums]` an existing section. If a type has no overlay set, the game crashes whenever it identifies a Tiberium overlay of a later slot, or one that belongs to no type. Loading a map that contains such an overlay is enough.
:::

## Cell state

A cell holds at most one Tiberium overlay and a growth stage from 0 to 11.

An overlay with [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) gives its cell the `Tiberium` [land type](/reference/enums/land-type/) when the overlay's [`Land`](/keys/land/) is `Clear`. Otherwise the cell takes the overlay's land type. Harvesters look only for the `Tiberium` land type, so they cannot collect an overlay that sets any other.

A blossom tree's cell counts as holding the type the tree seeds, but it has no overlay, so no harvester can collect from it. [Other sources of Tiberium](#other-sources-of-tiberium) covers the tree.

When a scenario loads, the engine replaces the stage of every Tiberium cell. The new stage depends on how many of the cell's eight neighbors hold the same type, so the stages stored in the map do not survive:

| Neighbors with the same type | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Stage | 0 | 1 | 3 | 4 | 6 | 7 | 8 | 10 | 11 |

A cell shows the frame for its stage from one of its type's overlays. The cell's map position picks that overlay, not the overlay the cell actually holds. Flat cells use the set's flat overlays and sloped cells use its slope overlays. The type's [`Color`](/keys/color/#scope-tiberium) recolors the result.

A type whose set has no slope overlays shows nothing on a slope. Only a map can place such a type there. A cell whose stage has no frame in the chosen shape is not drawn either.

## Growth

Each type runs growth passes and spread passes on separate timers, counted in [frames](/glossary/#frame). Both kinds of pass stop while the scenario's [`TiberiumGrowthEnabled`](/keys/tiberiumgrowthenabled/) switch is off. The [Tiberium growth](/mapping/actions/taction-tib-growth/) trigger action turns the switch on and off during play.

A growth pass runs every [`Growth`](/keys/growth/) frames. The delay restarts after each pass, whether or not anything grew.

:::note[TiberiumGrows only shortens the delay]
[`TiberiumGrows=yes`](/keys/tiberiumgrows/#scope-scenarios) cuts the growth delay to 30% of `Growth`. Growth still runs at the full delay with it off; only `TiberiumGrowthEnabled=no` stops growth. The key page explains when the scenario's setting applies.
:::

Each type keeps a queue of its cells that are still below stage 11. A pass takes a random number of cells from the front of that queue, at least 1. The upper limit is the queue length multiplied by [`GrowthPercentage`](/keys/growthpercentage/), raised to 5 if lower and cut to 50 if higher. A type with `GrowthPercentage` below `0.00001` never grows.

Each cell taken gains one stage if it still holds this type's Tiberium. A cell that has since lost its Tiberium or changed type still uses up one of the pass's cells, and leaves the queue.

After it grows, a cell below stage 11 goes back into the queue, near the back, and is queued to [spread](#spread) if it can. A cell that reaches stage 11 leaves the growth queue.

The queue is rebuilt from the map when a scenario starts and when a saved game loads. During play, it is also rebuilt once it has taken in about as many entries as the map has cells.

A cell the map stores at stage 11 does not grow at scenario start, even if its stage is lowered when the scenario loads. It joins the queue at the next rebuild.

:::caution[A full-grown cell that is cut back stops growing]
When harvesting, a crater or warhead damage lowers a stage-11 cell without clearing it, the cell does not rejoin the growth queue. Placing more Tiberium on the cell does not queue it either. It grows again only after the next rebuild. A [chain reaction](#damage) is the exception: it queues the cell again.
:::

## Spread

A spread pass runs every [`Spread`](/keys/spread/#scope-tiberium) frames; `TiberiumGrows` does not shorten this delay. Each type keeps a queue of cells that may spread. A pass seeds from cells at the front of that queue until it has spent a random budget, at least 1. The upper limit is the queue length multiplied by [`SpreadPercentage`](/keys/spreadpercentage/), raised to 5 if lower and cut to 25 if higher.

A cell can spread only when all of the following hold, tested in this order:

1. [`TiberiumSpreads=yes`](/keys/tiberiumspreads/) is in force;
2. the cell still holds Tiberium;
3. its stage is above the ripeness threshold of the type it holds;
4. that type's `SpreadPercentage` is at least `0.00001`;
5. nothing is standing in the cell.

:::caution[The ripeness threshold comes from the slot]
A cell must be above half its type's slot number, rounded down. Types in slots 0 and 1 spread from stage 1, and types in slots 2 and 3 only from stage 2. Reordering `[Tiberiums]` therefore changes how ripe a field must be before it spreads.
:::

A spreading cell starts at a random neighbor and checks all eight in turn. It seeds the first neighbor that accepts growth, and the new cell starts at stage 5.

A neighbor accepts growth when all of the following hold, tested in this order:

1. it lies inside the playable area;
2. it is not under a bridge and has never been under one;
3. it holds no building with strength left, unless the building's type is [`Invisible`](/keys/invisible/) or [`InvisibleInGame`](/keys/invisibleingame/), so that growth does not outline a hidden structure;
4. it holds no [`SpawnsTiberium=yes`](/keys/spawnstiberium/) terrain object, so a blossom tree's cell stays bare;
5. its land type is [`Buildable=yes`](/keys/buildable/);
6. it has no overlay, so veins, walls, crates and existing Tiberium all block it;
7. it is flat or on one of the four simple slopes, and it is flat if the type's overlay set has no slope overlays;
8. its tile set is [`AllowTiberium=yes`](/keys/allowtiberium/).

The four simple slopes each raise two adjacent corners of the cell. Corner, steep and double slopes never accept Tiberium. A Tiberium overlay on one of them is removed when the cell's terrain is next recalculated, for example when the map loads.

Only a cell that has a neighbor accepting growth spends budget. A cell with none, or one that fails the spread tests, leaves the queue without spending any.

A cell that had more than one free neighbor goes straight back to the front of the queue, so it usually seeds again in the same pass. A cell that seeds its last free neighbor is not queued to spread again until the queue is rebuilt, even if a neighbor is later cleared. The spread queue is rebuilt at the same times as the growth queue.

A type whose set has no slope overlays can waste its passes. When the engine counts a cell's free neighbors, it counts a sloped neighbor as free for every type, but seeding then refuses the slope for this type. A cell whose only free neighbors are sloped spends budget and seeds nothing. With two or more such neighbors it returns to the front of the queue each time, so it can use up the whole pass.

## Harvesting

A UnitType with [`Harvester=yes`](/keys/harvester/#scope-unittype) harvests Tiberium on its own. It starts harvesting when it is placed on the map, including when it leaves a factory or a repair bay. When it later goes idle, it resumes harvesting if its house is computer-controlled or it is standing on Tiberium.

A player-owned harvester that goes idle anywhere else takes a guard mission instead, so a player can park it. An armed harvester behaves the same way, except that it may take area guard instead of guard.

A harvester whose house owns none of the buildings in its [`Dock`](/keys/dock/) list, or whose list is empty, switches to guard.

A vehicle with neither `Harvester=yes` nor [`Weeder=yes`](/keys/weeder/#scope-unittype) that is given the harvest mission stands still.

:::caution[Do not set both Weeder and Harvester]
A vehicle with both `Weeder=yes` and `Harvester=yes` looks for veins when it sets out, but loads only on Tiberium ground. Each cycle there, it adds one or two units to its slot 0 compartment and leaves the cell's Tiberium untouched.
:::

### Finding a patch

A harvester that is not full first drives back to the patch it recorded on its previous trip, if it has one. Otherwise it searches for Tiberium out to one cell less than [`TiberiumFarScan`](/keys/tiberiumfarscan/).

The search takes the cell the harvester stands in if it is Tiberium ground. Otherwise it checks rings of cells at increasing distance and takes the most valuable cell in the nearest ring that has any. A cell's value is its type's [`Value`](/keys/value/) multiplied by its stage plus one.

A cell is skipped when any of the following holds, tested in this order:

1. it lies outside the playable area;
2. the game is a campaign, the local player owns the harvester, and the cell is shrouded;
3. it is in a different [movement zone](/glossary/#movement-zone) from the cell the harvester is heading to, or from the cell it stands in when it is not moving;
4. the harvester cannot enter it, or it is not Tiberium ground.

A computer-controlled harvester in a skirmish or multiplayer game searches differently when it sets out with no recorded patch. It checks every ring out to the limit instead of stopping at the first ring that has Tiberium. Along each side of a ring, it considers only the first cell of each unbroken run of Tiberium, which spreads the candidates across the field. It then picks one candidate at random, weighted by value and distance.

A candidate's weight is its value divided by a distance factor, and never less than 1. The factor is twice the ring's distance divided by the number of harvesters the house owns, rounded down and at least 1. Richer and nearer cells are therefore favored, and distance matters less the more harvesters the house owns. The count includes every type in [`HarvesterUnit`](/keys/harvesterunit/), whatever the searching vehicle's type.

If the search finds nothing and the harvester has nowhere to drive, it waits 7 seconds and then takes a guard mission. A harvester parked on a refinery first moves off it.

A `Harvester=yes` vehicle that gives up this way marks its house short of Tiberium for the rest of the scenario. A computer-controlled house marked this way stops [building replacement harvesters](/keys/harvester/#scope-global-rules). Its harvesters that go to guard also stay there instead of resuming harvesting.

### Loading

A harvester standing on Tiberium ground lifts one growth stage every 9 times [`HarvesterLoadRate`](/keys/harvesterloadrate/) frames and stores it as one unit. The unit goes into the compartment of the cell's type, so a harvester that works a mixed field carries a mixed load.

Each lift lowers the cell by one stage. A cell at stage 0 is cleared to bare ground by the next lift, which gives nothing, so a cell at stage 11 yields 11 units, not 12.

When the cell runs out before the harvester is full, the harvester searches out to one cell less than [`TiberiumNearScan`](/keys/tiberiumnearscan/) for the next one. If it finds none, it heads home with a partial load.

A harvester that fills its [`Storage`](/keys/storage/) runs the same search out to one cell less than `TiberiumNearScan`, records the cell it finds as the patch to return to, and heads home.

### Unloading

A harvester returning on its own looks for a bay among its house's buildings whose types are in its [`Dock`](/keys/dock/) list and that it can reach. It takes the nearest bay across all the listed types. Within one type, the house's primary building beats every bay of that type built before it, however near. A bay built after the primary still wins if it is nearer than the primary.

A building accepts a harvester only when all of the following hold:

1. it is [`DockUnload=yes`](/keys/dockunload/);
2. its house and the harvester's house are each allied with the other, which a house always is with itself;
3. it is not being built or sold, and it has not been switched off;
4. no other vehicle is docked there or on its way in.

A player can also order a harvester into an allied house's refinery. Rule 2 still applies, so the alliance must run both ways.

A harvester does not always pick a free bay. It compares the nearest free bay with the nearest bay of any kind. If the free bay is farther by more than the distance the harvester could drive while waiting, it drives to the busy one and waits in line.

The wait counts what the vehicle at that building still has to unload, that vehicle's drive in if it has not arrived yet, and every load already waiting in line there. Waiting reserves nothing: the harvester chooses again when it arrives, by which time the bay may be free or the line longer.

A docked harvester turns to face east, and the building west of it plays its pre-production animation. The harvester then hands its house one unit every [`HarvesterDumpRate`](/keys/harvesterdumprate/) minutes of game time, starting with its lowest slot.

Once it is empty, the harvester waits for a [`Refinery=yes`](/keys/refinery/) building west of it to finish its production animation. It then resumes harvesting, unless the player has given it another order.

A player order that sends the harvester elsewhere while it unloads ends the unload early. The harvester still waits for the refinery's animation, then leaves with the units it has not handed over.

## Credits and storage

Each unit a harvester unloads adds five points to its house's score.

A computer-controlled house in a skirmish or multiplayer game converts each unit to credits at once, at its type's [`Value`](/keys/value/). Its storage capacity does not matter.

Every other house stores each unit. Units beyond the house's free storage capacity are lost. The rest fill the house's standing buildings that declare [`Storage`](/keys/storage/), one building at a time.

Stored units become credits only when the house spends. Spending uses loose credits first, then stored units one at a time. Each building gives up its units lowest slot first, before the next building is used. Each unit is priced at its type's `Value` when it is spent, not when it is harvested.

A captured building keeps its contents. They move from the old house's total to the new house's total, along with the building's capacity.

A destroyed building scatters its contents one unit at a time, one to three cells from where it stood. Each unit starts stage 1 Tiberium of its type where the ground accepts growth.

On a cell that already holds that type, a unit adds one stage if the cell is below stage 11, `TiberiumGrowthEnabled` is on and the type's `GrowthPercentage` is at least `0.00001`. Any other unit is lost.

A sold building hands its contents back to its house after the building's capacity has been removed. The house's remaining storage buildings take what fits and the rest is lost, so a house whose only storage was the sold building loses all of it. A computer-controlled house in a skirmish or multiplayer game converts the whole store to credits instead.

A building with [`SiloDamage=yes`](/keys/silodamage/) shows how full it is with an extra animation. The animation does not appear while the building is less than one-eighth full.

## Damage

Infantry take damage each time they finish moving into a Tiberium cell. The damage is the Tiberium type's [`Power`](/keys/power/#scope-tiberium) divided by ten, rounded down, and at least 1. Armor, veterancy and other damage reductions do not lower it.

Infantry whose type sets [`TiberiumProof=yes`](/keys/tiberiumproof/), or that have the [`TIBERIUM_PROOF`](/systems/veterancy/#abilities) veteran ability, take none. When the scenario sets [`TiberiumDeathToVisceroid=yes`](/keys/tiberiumdeathtovisceroid/), infantry killed this way leave a small visceroid owned by the Neutral house, unless a vehicle stands in the cell.

An overlay with [`ChainReaction=yes`](/keys/chainreaction/) lets explosions set off the Tiberium in its cell. For a Tiberium overlay, the explosion's warhead must set [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype). A sonic wave sets the cell off without that warhead test.

A cell detonates only at stage 2 or higher, with a chance of 5% per stage. The detonation removes half the cell's stages, rounded down. It deals that many stages multiplied by `Power` as damage, through the [`C4Warhead`](/keys/c4warhead/).

After a detonation, each neighboring Tiberium cell above stage 2 has an 80% chance of being hit 1 to 8 seconds later by the `INVISO` animation. With the shipped rules, that hit can set off a neighbor whose overlay has `ChainReaction=yes` in turn. A type with `Power=0` shows no explosion and deals no damage, but its detonation still removes the stages and still hits neighbors.

On an overlay with `ChainReaction=yes`, an explosion that passes the warhead test also removes one stage for every ten points of its damage, whether or not the cell detonates. Explosions never thin or set off a cell whose overlay lacks it. In the shipped rules, the overlays of the `Image=1` set do not set it.

An animation with [`TiberiumChainReaction=yes`](/keys/tiberiumchainreaction/) that starts on a Tiberium cell clears the cell and deals [`TiberiumExplosionDamage`](/keys/tiberiumexplosiondamage/) through the `C4Warhead`. One time in three, it also leaves one of the type's [`Debris`](/keys/debris/) animations, recolored with the type's [`Color`](/keys/color/#scope-tiberium).

With [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules) in `[CombatDamage]`, a destroyed vehicle carrying Tiberium explodes over a radius of one and a half cells. The damage is the sum, over its compartments, of the amount held multiplied by that type's `Power`. A scenario with [`HarvesterImmune=yes`](/keys/harvesterimmune/) prevents the blast. [A vehicle](/systems/destruction-and-debris/#a-vehicle) covers when it goes off.

Some effects remove stages outright. An animation with [`Crater=yes`](/keys/crater/#scope-animtype) that plays at ground level removes six stages from its cell. If it also sets [`Scorch=yes`](/keys/scorch/), it does so only half the time. Laying a [laser fence](/systems/laser-fences/) clears every cell along the run.

## Other sources of Tiberium

A blossom tree is a terrain object with [`SpawnsTiberium=yes`](/keys/spawnstiberium/) and [`IsAnimated=yes`](/keys/isanimated/). While it is idle, it starts its animation on each frame with the chance set by [`AnimationProbability`](/keys/animationprobability/), and plays it at [`AnimationRate`](/keys/animationrate/).

When the animation reaches the halfway frame of the tree's shape file, the animation stops and the tree seeds one neighboring cell. The seed uses the type in slot [`TiberiumToSpawn`](/keys/tiberiumtospawn/), starts at stage 5, and must pass the neighbor tests under [Spread](#spread). The tree seeds even while `TiberiumSpreads=no` or `TiberiumGrowthEnabled=no`.

A Tiberium [crate](/systems/crates/) places stage 1 Tiberium of a randomly chosen registered type. When the choice is slot 1, it uses slot 0 instead.

A VoxelAnimType with [`IsTiberium=yes`](/keys/istiberium/#scope-voxelanimtype) seeds stage 0 Tiberium where it comes down, unless it lands in water or on a bridge. With [`IsMeteor=yes`](/keys/ismeteor/#scope-voxelanimtype) it seeds the eight cells around the impact instead of the impact cell. The type is always the first registered type with `Image=3`, or slot 0 if no type uses that set.

A destroyed object whose type sets [`TiberiumHeal=yes`](/keys/tiberiumheal/) seeds slot 0 Tiberium around itself, and a destroyed harvester can spill its load as slot 0 Tiberium. [Destruction and debris](/systems/destruction-and-debris/#spilled-harvester-loads) covers both.

## Settings the engine parses but never reads

The following settings have no effect:

- [`TiberiumGrows` in `[MultiplayerDefaults]`](/keys/tiberiumgrows/#scope-global-rules). The scenario's `[SpecialFlags]` entry is the one that works.
- [`TiberiumExplosive` in `[SpecialFlags]`](/keys/tiberiumexplosive/#scope-scenarios). Use the `[CombatDamage]` entry.
- [`TiberiumStrength`](/keys/tiberiumstrength/) in `[CombatDamage]`.
- [`TiberiumTransmogrify`](/keys/tiberiumtransmogrify/) in `[General]`.
