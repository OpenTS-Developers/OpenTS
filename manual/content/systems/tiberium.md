---
title: Tiberium
summary: "Grows, spreads, and harvests the registered Tiberium types across map cells."
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
0=MyTiberium ; example Tiberium type, registered in slot 0

[MyTiberium]
Image=1
Value=25
Power=10
Growth=8
GrowthPercentage=.02
Spread=20
SpreadPercentage=.02
```

Each entry in `[Tiberiums]` names a rules section that supplies the type's settings. An entry whose own number is below the count of types registered so far re-reads the type already in that Tiberium slot. Every other entry creates a new type in the next free slot, so with the conventional `0=` through `3=` list the entry numbers and the slots coincide. A named section that does not exist leaves the type on its built-in values, which include no overlay set at all.

The slot selects the storage compartment a harvested load occupies, and it raises the growth stage a cell must reach before it spreads.

:::danger[Four slots exist]
A house and a harvester each track exactly four Tiberium compartments. A fifth registered type is given slot 4, and every deposit or withdrawal made for it writes past the end of that record.
:::

[`Image`](/keys/image/#scope-tiberium) selects the overlay set, and through it two runtime limits: the number of growth stages the set has, and whether the type is allowed onto sloped ground.

:::caution[The large-Tiberium set has a single stage]
[`Image=2`](/keys/image/#scope-tiberium) gives the type one growth stage. Growth requires a cell below the last stage, and seeding a bare cell places stage 5, so a type on that set can neither grow nor be created by spreading. It appears only where the map or a third-party editor puts it, or, for the first registered type, where a destroyed `TiberiumHeal=yes` object spews it.
:::

:::danger[Every registered type needs an overlay set]
Identifying the Tiberium in a cell walks the registered types in order and reads each type's overlay set. A type left without one ends that walk in a null read: either it has no [`Image`](/keys/image/#scope-tiberium) at all, or its section is missing. The read fails as soon as a cell has an overlay belonging to a later type.
:::

## Cell state

A cell holds one Tiberium overlay and one growth stage from 0 through 11. The cell is worth [`Value`](/keys/value/) multiplied by the stage plus one, so a ripe cell of a twelve-stage set is worth twelve times the setting. A blossom tree is a terrain object that seeds Tiberium into the ground beside it. It sits on a cell that reports the type it seeds but is worth nothing, because worth comes from the overlay and that cell has none. The [other sources](#other-sources-of-tiberium) section covers the seeding.

An overlay declared [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) whose own land type is clear gives its cell the `Tiberium` [land type](/reference/enums/land-type/), and that land type, not the overlay, is what every harvesting test reads.

When a scenario finishes loading, and again whenever a Tiberium overlay is placed onto the map directly, the cell's stage is replaced by a smoothing lookup. That lookup counts the cell's eight neighbors that hold the same type. On a twelve-stage set, the stage rises with that count, one value per neighbor from 0 through 8: 0, 1, 3, 4, 6, 7, 8, 10, 11. A stage stored in the map file does not survive that pass.

Drawing a cell uses that stage as a frame number in one of the type's overlays. Which one is settled by the cell's own coordinates, so a field does not repeat itself: a flat cell draws from the set's flat overlays and a sloped cell from its slope overlays. A type whose set has no slope overlays draws nothing on a slope, which is ground only a map or a third-party editor can put its Tiberium on. If the selected SHP does not have the stage's frame, the overlay is omitted from both the tactical view and its redraw rectangle instead of reading beyond the artwork's frame table.

## Growth

The main game logic offers every type a growth pass and then a spread pass each frame. Both are gated by the scenario's [`TiberiumGrowthEnabled`](/keys/tiberiumgrowthenabled/) switch, which the [Tiberium growth](/mapping/actions/taction-tib-growth/) trigger action turns on and off during play.

A type takes a growth pass when its timer runs out; the timer is then reloaded with [`Growth`](/keys/growth/) [frames](/glossary/#frame) whether or not anything grew.

:::note[TiberiumGrows shortens the wait, it does not enable growth]
[`TiberiumGrows=yes`](/keys/tiberiumgrows/#scope-scenarios) multiplies the reloaded growth delay by `0.3`. Growth with the flag off runs at the full delay; only `TiberiumGrowthEnabled=no` stops it.
:::

Each type keeps a queue of cells that have not finished ripening. A pass first sizes a budget as the queued count multiplied by [`GrowthPercentage`](/keys/growthpercentage/), clamped to between 5 and 50. It then draws a random figure from 1 up to that budget and processes that many entries. Every entry taken counts against the budget, including one whose cell has since changed type and is simply dropped.

A cell gains one stage when all of the following hold, tested in this order:

1. the scenario's growth switch is on;
2. the cell still holds Tiberium of a registered type;
3. its stage is below the last one the overlay set has;
4. that type's `GrowthPercentage` is at least `0.00001`.

A cell still short of stage 11 goes back into the queue with a delay of up to 49 frames, and the same pass offers it to the spread queue. A cell that has reached stage 11 is dropped from growth entirely.

:::caution[A harvested full-grown cell stops growing back]
Removing stages from a cell standing at stage 11 tries to put it back into the growth queue before the removal, while the cell is still full, so the attempt is refused. The partly harvested cell then sits below the ceiling with no queue entry, and placing more Tiberium on it re-queues only its spreading. It re-enters the growth queue only when the queue is rebuilt, which happens when a scenario starts, when a saved game is loaded, and whenever the queue runs short of room for new entries.
:::

## Spread

Spread passes are scheduled the same way, from the [`Spread`](/keys/spread/#scope-tiberium) delay, and no flag shortens them. The budget is the queued count multiplied by [`SpreadPercentage`](/keys/spreadpercentage/), clamped to between 5 and 25, with a random figure drawn from 1 up to it. Only a cell whose census finds a free neighbor counts against that budget. One hemmed in on all eight sides is dropped without spending any of it, and so is a cell that no longer passes the spread tests at all. A cell with more than one free neighbor goes back into the queue for the next pass.

A cell may spread when all of the following hold, tested in this order:

1. [`TiberiumSpreads=yes`](/keys/tiberiumspreads/) is in force;
2. the cell still holds Tiberium of a registered type;
3. its stage clears that type's ripeness threshold;
4. that type's `SpreadPercentage` is at least `0.00001`;
5. nothing is standing in the cell.

:::caution[The ripeness threshold comes from the type's slot]
The stage a cell must exceed is half the type's slot number, rounded down: types in slots 0 and 1 spread from stage 1, and types in slots 2 and 3 only from stage 2. Reordering `[Tiberiums]` therefore changes how ripe a field must be before it creeps.
:::

The source cell picks a random starting facing, walks all eight neighbors from there, and seeds the first that accepts growth. A newly seeded cell starts at stage 5, and is given an overlay drawn at random from the type's own set.

A neighboring cell accepts growth when all of the following hold, tested in this order:

1. it lies inside the playable area;
2. it is not under a bridge and never has been;
3. it holds no building with strength left, unless that building's type is invisible, which keeps growth from outlining a hidden structure;
4. it holds no [`SpawnsTiberium=yes`](/keys/spawnstiberium/) terrain object, which is what keeps a blossom tree's own cell bare;
5. its land type is [`Buildable=yes`](/keys/buildable/);
6. it has no overlay at all, so veins, walls, crates and existing Tiberium all block it;
7. it is flat, or has one of the four standard ramps, and a type whose overlay set has no ramp frames refuses every slope; and
8. its theater tile set is [`AllowTiberium=yes`](/keys/allowtiberium/).

The four standard ramps of rule 7 are the slopes that fall away toward one of the map's four directions, raising two of the cell's corners. The corner, steep and double ramp shapes lie outside that set. A Tiberium overlay found on one of those other slopes is removed outright the next time the cell's attributes are recalculated.

The census that decides whether a cell is worth a pass runs that same list, and it is where a wasted pass comes from. The census puts the question without naming a type, so it skips the second half of rule 7, the refusal that a type whose overlay set has no ramp frames applies to every slope. A sloped neighbor therefore counts as free. The seeding walk then puts the question again with the type in hand, and that refusal runs. A type barred from slopes spends a pass on a cell whose only free neighbors are sloped, and seeds nothing.

## Harvesting

A UnitType with [`Harvester=yes`](/keys/harvester/#scope-unittype) takes the harvest mission on its own. It does so when it is first placed on the map, when it drives out of a factory or a repair bay, whenever its house is computer-controlled, and whenever it goes idle standing on Tiberium. A player-owned one that goes idle anywhere else takes guard instead, which is what lets a harvester be parked. Carrying a weapon changes none of this, only which guard mission it falls back to. A vehicle with neither that flag nor [`Weeder=yes`](/keys/weeder/#scope-unittype) given the same mission stands still for 30 seconds at a time. A harvester whose house owns no building named in its [`Dock`](/keys/dock/) list, including one with an empty list, is switched to guard.

:::caution[A vein harvester never lifts Tiberium]
A vehicle with both [`Weeder=yes`](/keys/weeder/#scope-unittype) and `Harvester=yes` takes the vein branch, while the eligibility test still reads the Tiberium branch. It waits for Tiberium ground, then loads one or two units into the first Tiberium compartment each cycle, and leaves the cell's stages untouched. A unit here is the counted quantity a compartment holds, not an object on the map.
:::

### Finding a patch

A harvester that is not full first heads back to the patch it recorded on its last trip, and otherwise searches out to [`TiberiumFarScan`](/keys/tiberiumfarscan/). The plain search takes the harvester's own cell when that is already Tiberium ground; otherwise it walks outward one ring at a time and takes the richest qualifying cell in the first ring that yields any. A cell is skipped when any of these holds, tested in this order:

1. it lies outside the playable area;
2. the match is a campaign, the local player owns the harvester, and the cell is shrouded;
3. it sits in a different [movement zone](/glossary/#movement-zone) from the harvester's destination;
4. the harvester could not enter it, or it is not Tiberium ground.

A computer-controlled harvester in a skirmish or multiplayer game uses a weighted search instead. It scans every ring out to the limit, and offers only the first cell of each unbroken run along a side so that the candidates spread across the field. From those it draws one at random, with a weight of at least 1 taken from the cell's worth divided by the ring's span per harvester the house owns, treating that divisor as at least 1. That count covers every type named in [`HarvesterUnit`](/keys/harvesterunit/), whatever the searching vehicle's own type is.

With no patch and nowhere to go, the harvester is marked useless, its house is flagged short of Tiberium, and it retries after 7 seconds. The idle branch then picks the repair bay when the house owns one and hunt otherwise, but assigns the guard mission after that choice, so the harvester guards.

### Loading

Harvesting lifts one growth stage per cycle, and a cycle is nine stage ticks of [`HarvesterLoadRate`](/keys/harvesterloadrate/) frames each. The stage is credited to the compartment of the cell's own type, so a harvester crossing a mixed field comes home with a mixed load, and taking the last stage clears the cell to bare ground. A harvester that has filled its [`Storage`](/keys/storage/) records a patch found within [`TiberiumNearScan`](/keys/tiberiumnearscan/) as the patch to return to, and heads home.

### Unloading

The harvester asks every [`Dock`](/keys/dock/) building type for a bay among its own house's buildings and takes the nearest that answers. Within a single type the house's primary building wins whenever it answers, however far off it stands; across types the shorter trip wins. A building answers only while it is [`DockUnload=yes`](/keys/dockunload/) and has nothing else attached. A harvester directed at another house's refinery may enter the bib and dock only when each house declares the other an ally. A one-way alliance grants neither permission, and `Dock` remains a return-target list rather than a compatibility flag.

A free bay is not always the choice. When the nearest free one is further off than the nearest occupied one by more than the harvester could drive in the time that queue will take, it drives to the occupied one and waits. The wait counts what the harvester at the building has left to hand over, that harvester's own trip in if it has not arrived yet, and every load already waiting there. Waiting reserves nothing, so the choice is made again on arrival, by which time the bay may be free or the line longer.

On arrival the harvester turns to face east, and the building west of it runs its pre-production animation. One stored unit is then handed to the house every [`HarvesterDumpRate`](/keys/harvesterdumprate/) minutes' worth of frames. An emptied harvester waits for a [`Refinery=yes`](/keys/refinery/) building west of it to finish its production animation before taking the harvest mission again.

## Credits and storage

Each unit handed over adds five to the house's score. A computer house in a skirmish or multiplayer game then converts the unit straight to credits at its type's [`Value`](/keys/value/), with no reference to storage capacity at all.

Every other house stores it. The amount is first clipped to the capacity the house's buildings still have free, and the surplus is discarded rather than credited. What remains is distributed one unit at a time into the house's standing buildings that declare [`Storage`](/keys/storage/), filling each in turn. Spending drains loose credits first and only then draws stored units, one at a time from the lowest occupied compartment and each converted at that compartment's type's `Value`. A store is therefore priced when it is spent rather than when it is filled.

A captured building keeps its contents: they leave the old house's total and join the new one's, along with the building's capacity. A destroyed building scatters them across the surrounding cells one unit at a time, each landing as a stage-1 patch wherever the ground accepts growth. A sold building hands them back to its own house along the route a harvester unload takes, after the sale has already withdrawn that building's own capacity. The house's remaining storage buildings take what they can, and the surplus is discarded, so a house whose only storage was the building it sold keeps none of it. A computer house in a skirmish or multiplayer game banks the whole store as credits instead. A building declaring [`SiloDamage=yes`](/keys/silodamage/) draws its fill level over itself, and shows nothing while it is empty.

## Damage

Infantry stepping onto a Tiberium cell, including a blossom tree's cell, takes [`Power`](/keys/power/#scope-tiberium) divided by ten, never less than 1. The damage is skipped when the type declares [`TiberiumProof=yes`](/keys/tiberiumproof/) or the object holds the Tiberium-proof veteran ability. A death caused this way spawns a small visceroid for the Neutral house when the scenario declares [`TiberiumDeathToVisceroid=yes`](/keys/tiberiumdeathtovisceroid/).

An overlay declaring [`ChainReaction=yes`](/keys/chainreaction/) lets damage set off the Tiberium in its cell, once the cell holds at least the second growth stage. A Tiberium overlay detonates under a warhead declaring [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype), or under a sonic wave, which skips the warhead test. The chance is five times the cell's stage. The blast consumes half the stage and deals that many stages multiplied by `Power`, and each of the eight neighbors above stage 2 has an 80% chance of a delayed detonation of its own. A zero result draws no explosion and deals no damage, but it still consumes the growth and performs those neighboring checks. An animation declaring [`TiberiumChainReaction=yes`](/keys/tiberiumchainreaction/) clears the cell it sits on outright, applies [`TiberiumExplosionDamage`](/keys/tiberiumexplosiondamage/), and one time in three leaves one of the type's [`Debris`](/keys/debris/) animations recolored by its [`Color`](/keys/color/#scope-tiberium).

With [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules) a destroyed vehicle carrying Tiberium explodes over one and a half cells, unless the scenario declares [`HarvesterImmune=yes`](/keys/harvesterimmune/). The damage is the sum, across every compartment, of the amount held there multiplied by that compartment's type `Power`. Crater-forming animations strip six stages from the cell they land on, and a laser fence clears twelve from every cell along its span.

## Other sources of Tiberium

A blossom tree is a terrain object with [`SpawnsTiberium=yes`](/keys/spawnstiberium/) and an animation. It seeds a neighboring cell at the midpoint of that animation, skipping the source-cell test entirely, with the type named by its [`TiberiumToSpawn`](/keys/tiberiumtospawn/). A Tiberium crate picks a registered type at random, swapping slot 1 for slot 0, and lays a stage-1 patch at the crate plus another ten to twenty scattered around it. A VoxelAnimType declaring [`IsTiberium=yes`](/keys/istiberium/#scope-voxelanimtype) seeds Tiberium at stage 0 where it lands: the ring of eight cells around a meteor's impact, or the single cell beneath any other animation. The type is always the one that owns the second Tiberium overlay set. A destroyed object whose type declares [`TiberiumHeal=yes`](/keys/tiberiumheal/) leaves the type in slot 0 at stage 0 through 2 on its own cell and the four beside it.

## Settings the engine parses but never reads

[`TiberiumGrows` in `[MultiplayerDefaults]`](/keys/tiberiumgrows/#scope-global-rules) and [`TiberiumExplosive` in `[SpecialFlags]`](/keys/tiberiumexplosive/#scope-scenarios) are stored and never read; the spellings that work are the scenario `[SpecialFlags]` entry and the `[CombatDamage]` entry respectively. [`TiberiumStrength`](/keys/tiberiumstrength/) in `[CombatDamage]` and [`TiberiumTransmogrify`](/keys/tiberiumtransmogrify/) in `[General]` are stored and never read at all.
