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

Each entry in `[Tiberiums]` names a rules section that supplies the type's settings. An entry whose own number is below the count of types registered so far re-reads the type already in that Tiberium slot; every other entry creates a new type in the next free slot, so with the conventional `0=` through `3=` list the entry numbers and the slots coincide. A named section that does not exist leaves the type on its built-in values, which include no overlay set at all.

The slot selects the storage compartment a harvested load occupies, and it raises the growth stage a cell must reach before it spreads.

:::danger[Four slots exist]
A house and a harvester each track exactly four Tiberium compartments. A fifth registered type is given slot 4, and every deposit or withdrawal made for it writes past the end of that record.
:::

[`Image`](/keys/image/#scope-tiberium) selects the overlay set, and through it two runtime limits: the number of growth stages the set carries, and whether the type is allowed onto sloped ground.

:::caution[The large-Tiberium set has a single stage]
[`Image=2`](/keys/image/#scope-tiberium) gives the type one growth stage. Growth requires a cell below the last stage, and seeding a bare cell places stage 5, so a type on that set can neither grow nor be created by spreading; it appears only where the map or an editor puts it — or, for the first registered type, where a destroyed `TiberiumHeal=yes` object spews it.
:::

:::danger[Every registered type needs an overlay set]
Identifying the Tiberium in a cell walks the registered types in order and reads each type's overlay set. A type left without one — no [`Image`](/keys/image/#scope-tiberium), or no section — ends that walk in a null read as soon as a cell carries an overlay belonging to a later type.
:::

## Cell state

A cell holds one Tiberium overlay and one growth stage from 0 through 11. The cell is worth [`Value`](/keys/value/) multiplied by the stage plus one, so a ripe cell of a twelve-stage set is worth twelve times the setting. A blossom tree — a terrain object that seeds Tiberium into the ground beside it, described under [other sources](#other-sources-of-tiberium) — sits on a cell that reports the type it seeds but is worth nothing, because worth comes from the overlay and that cell carries none.

An overlay declared [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) whose own land type is clear gives its cell the `Tiberium` [land type](/reference/enums/land-type/), and that land type — not the overlay — is what every harvesting test reads.

When a scenario finishes loading, and again whenever a Tiberium overlay is placed onto the map directly, the cell's stage is replaced by a smoothing lookup on the number of its eight neighbors holding the same type, running on a twelve-stage set from stage 0 with no matching neighbor up to stage 11 with all eight. A stage stored in the map file does not survive that pass.

Drawing a cell uses that stage as a frame number in the selected flat or slope overlay. If the selected SHP does not carry that frame, the overlay is omitted from both the tactical view and its redraw rectangle instead of reading beyond the artwork's frame table.

## Growth

The main game logic offers every type a growth pass and then a spread pass each frame. Both are gated by the scenario's [`TiberiumGrowthEnabled`](/keys/tiberiumgrowthenabled/) switch, which the [Tiberium growth](/mapping/actions/taction-tib-growth/) trigger action turns on and off during play.

A type takes a growth pass when its timer runs out; the timer is then reloaded with [`Growth`](/keys/growth/) frames whether or not anything grew.

:::note[TiberiumGrows shortens the wait, it does not enable growth]
[`TiberiumGrows=yes`](/keys/tiberiumgrows/#scope-scenarios) multiplies the reloaded growth delay by `0.3`. Growth with the flag off runs at the full delay; only `TiberiumGrowthEnabled=no` stops it.
:::

Each type keeps a queue of cells that have not finished ripening. A pass first sizes a budget as the queued count multiplied by [`GrowthPercentage`](/keys/growthpercentage/), clamped to between 5 and 50, then draws a random figure from 1 up to that budget and processes that many entries. Every entry taken counts against the budget, including one whose cell has since changed type and is simply dropped.

A cell gains one stage when all of the following hold, tested in this order:

1. the scenario's growth switch is on;
2. the cell still holds Tiberium of a registered type;
3. its stage is below the last one the overlay set carries;
4. that type's `GrowthPercentage` is at least `0.00001`.

A cell still short of stage 11 goes back into the queue with a delay of up to 49 frames and is offered to the spread queue at the same time; a cell that has reached stage 11 is dropped from growth entirely.

:::caution[A harvested full-grown cell stops growing back]
Removing stages from a cell standing at stage 11 tries to put it back into the growth queue before the removal, while the cell is still full, so the attempt is refused. The partly harvested cell then sits below the ceiling with no queue entry — placing more Tiberium on it re-queues only its spreading — and it re-enters the growth queue only when the queue is rebuilt.
:::

## Spread

Spread passes are scheduled the same way, from the [`Spread`](/keys/spread/#scope-tiberium) delay, and no flag shortens them. The budget is the queued count multiplied by [`SpreadPercentage`](/keys/spreadpercentage/), clamped to between 5 and 25, with a random figure drawn from 1 up to it. Only a cell that finds somewhere to seed counts against that budget. A cell hemmed in on all eight sides is dropped from the queue without spending any of it, as is one that no longer carries Tiberium of its own; a cell with more than one free neighbor is re-queued to run again on the next pass.

A cell may spread when all of the following hold, tested in this order:

1. [`TiberiumSpreads=yes`](/keys/tiberiumspreads/) is in force;
2. the cell still holds Tiberium of a registered type;
3. its stage clears that type's ripeness threshold;
4. that type's `SpreadPercentage` is at least `0.00001`;
5. nothing is standing in the cell.

:::caution[The ripeness threshold comes from the type's slot]
The stage a cell must exceed is half the type's slot number, rounded down: types in slots 0 and 1 spread from stage 1, and types in slots 2 and 3 only from stage 2. Reordering `[Tiberiums]` therefore changes how ripe a field must be before it creeps.
:::

The source cell picks a random starting facing, walks all eight neighbors from there, and seeds the first that accepts growth. A newly seeded cell starts at stage 5.

A neighboring cell accepts growth when all of the following hold, tested in this order:

1. it lies inside the playable area;
2. it is not under a bridge and never has been;
3. it holds no building with strength left, unless that building's type is invisible — the exemption keeps growth from outlining a hidden structure;
4. it holds no [`SpawnsTiberium=yes`](/keys/spawnstiberium/) terrain object, which is what keeps a blossom tree's own cell bare;
5. its land type is [`Buildable=yes`](/keys/buildable/);
6. it carries no overlay at all, so veins, walls, crates and existing Tiberium all block it;
7. it is flat, or carries one of the four standard ramps, and a type whose overlay set carries no ramp frames refuses every slope; and
8. its theater tile set is [`AllowTiberium=yes`](/keys/allowtiberium/).

The four standard ramps of rule 7 are the slopes that fall away toward one of the map's four directions, raising two of the cell's corners; the corner, steep and double ramp shapes lie outside that set. A Tiberium overlay found on one of those other slopes is removed outright the next time the cell's attributes are recalculated.

The census that decides whether a cell is worth a pass runs that same list, and it is where a wasted pass comes from. The census puts the question without naming a type, so the second half of rule 7 — the refusal a type whose overlay set carries no ramp frames applies to every slope — is skipped, and a sloped neighbor counts as free. The seeding walk then puts the question again with the type in hand, and that refusal runs. A type barred from slopes therefore spends a pass on a cell whose only free neighbors are sloped, and seeds nothing.

## Harvesting

A UnitType with [`Harvester=yes`](/keys/harvester/#scope-unittype) takes the harvest mission on its own. A vehicle with neither that flag nor [`Weeder=yes`](/keys/weeder/#scope-unittype) given the same mission stands still for 30 seconds at a time, and a harvester whose house owns no building named in its [`Dock`](/keys/dock/) list — including a harvester with an empty list — is switched to guard.

:::caution[A vein harvester never lifts Tiberium]
A vehicle carrying [`Weeder=yes`](/keys/weeder/#scope-unittype) takes the vein branch even when it also carries `Harvester=yes`, while the eligibility test still reads the Tiberium branch. Such a vehicle waits for Tiberium ground, then loads one or two units of Tiberium — the counted quantity a compartment holds, not an object — into the first Tiberium compartment per cycle, and leaves the cell's stages untouched.
:::

### Finding a patch

A harvester that is not full first heads back to the patch it recorded on its last trip, and otherwise searches out to [`TiberiumFarScan`](/keys/tiberiumfarscan/). The plain search takes the harvester's own cell when that is already Tiberium ground; otherwise it walks outward one ring at a time and takes the richest qualifying cell in the first ring that yields any. A cell is skipped when any of these holds, tested in this order:

1. it lies outside the playable area;
2. the match is a campaign, the local player owns the harvester, and the cell is shrouded;
3. it sits in a different [movement zone](/glossary/#movement-zone) from the harvester's destination;
4. the harvester could not enter it, or it is not Tiberium ground.

A computer-controlled harvester in a skirmish or multiplayer game uses a weighted search instead. It scans every ring out to the limit, offers only the first cell of each unbroken run along a side so that the candidates spread across the field, and draws one at random with a weight of at least 1 taken from the cell's worth divided by the ring's span over the number of harvesters the house owns. That census counts the first entry of [`HarvesterUnit`](/keys/harvesterunit/) whatever the searching vehicle's own type is.

With no patch and nowhere to go, the harvester is marked useless, its house is flagged short of Tiberium, and it retries after 7 seconds; the idle branch sends it to the repair bay when the house owns one and to hunt otherwise.

### Loading

Harvesting lifts one growth stage per cycle, and a cycle is nine stage ticks of [`HarvesterLoadRate`](/keys/harvesterloadrate/) frames each. The stage is credited to the compartment of the cell's own type, so a harvester crossing a mixed field comes home with a mixed load, and taking the last stage clears the cell to bare ground. A harvester that has filled its [`Storage`](/keys/storage/) records the nearest patch within [`TiberiumNearScan`](/keys/tiberiumnearscan/) as the patch to return to, and heads home.

### Unloading

The harvester asks each [`Dock`](/keys/dock/) building type in turn for a bay among its own house's buildings, taking the nearest that answers or the house's primary building of that type whenever it answers; a building answers only while it is [`DockUnload=yes`](/keys/dockunload/) and has nothing else attached. A harvester directed at another house's refinery may enter the bib and dock only when each house declares the other an ally. A one-way alliance grants neither permission, and `Dock` remains a return-target list rather than a compatibility flag. On arrival the harvester turns to face east, the building west of it runs its pre-production animation, and one stored unit is handed to the house every [`HarvesterDumpRate`](/keys/harvesterdumprate/) minutes' worth of frames. An emptied harvester waits for a [`Refinery=yes`](/keys/refinery/) building west of it to finish its production animation before taking the harvest mission again.

## Credits and storage

Each unit handed over adds five to the house's score. A computer house in a skirmish or multiplayer game then converts the unit straight to credits at its type's [`Value`](/keys/value/), with no reference to storage capacity at all.

Every other house stores it. The amount is first clipped to the capacity the house's buildings still have free, and the surplus is discarded rather than credited; what remains is distributed one unit at a time into the house's standing buildings that declare [`Storage`](/keys/storage/), filling each in turn. Spending drains loose credits first and only then draws stored units, one at a time from the lowest occupied compartment and each converted at that compartment's type's `Value`. A store is therefore priced when it is spent rather than when it is filled.

A captured building keeps its contents: they leave the old house's total and join the new one's, along with the building's capacity. A destroyed building scatters them across the surrounding cells one unit at a time, each landing as a stage-1 patch wherever the ground accepts growth. A sold building hands them back to its own house along the route a harvester unload takes, after the sale has already withdrawn that building's own capacity: the house's remaining storage buildings take what they can and the surplus is discarded, so a house whose only storage was the building it sold keeps none of it, while a computer house in a skirmish or multiplayer game banks the whole store as credits. A building declaring [`SiloDamage=yes`](/keys/silodamage/) draws its fill level over itself, and shows nothing while it is empty.

## Damage

Infantry stepping onto a Tiberium cell — including a blossom tree's cell — takes [`Power`](/keys/power/#scope-tiberium) divided by ten, never less than 1, unless the type declares [`TiberiumProof=yes`](/keys/tiberiumproof/) or the object carries the Tiberium-proof veteran ability. A death caused this way spawns a small visceroid for the Neutral house when the scenario declares [`TiberiumDeathToVisceroid=yes`](/keys/tiberiumdeathtovisceroid/).

An overlay declaring [`ChainReaction=yes`](/keys/chainreaction/) lets damage set off the Tiberium in its cell, once the cell holds at least the second growth stage. A Tiberium overlay detonates under a warhead declaring [`Tiberium=yes`](/keys/tiberium/#scope-warheadtype) — or under a sonic wave, which skips the warhead test. The chance is five times the cell's stage; the blast consumes half the stage and deals that many stages multiplied by `Power`, and each of the eight neighbors above stage 2 has an 80% chance of a delayed detonation of its own. A zero result draws no explosion and deals no damage, but it still consumes the growth and performs those neighboring checks. An animation declaring [`TiberiumChainReaction=yes`](/keys/tiberiumchainreaction/) clears the cell it sits on outright, applies [`TiberiumExplosionDamage`](/keys/tiberiumexplosiondamage/), and one time in three leaves one of the type's [`Debris`](/keys/debris/) animations recolored by its [`Color`](/keys/color/#scope-tiberium).

With [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/#scope-global-rules) a destroyed vehicle carrying Tiberium explodes over one and a half cells. The damage is the sum, across every compartment, of the amount held there multiplied by that compartment's type `Power`. Crater-forming animations strip six stages from the cell they land on, and a laser fence clears twelve from every cell along its span.

## Other sources of Tiberium

A blossom tree — a terrain object with [`SpawnsTiberium=yes`](/keys/spawnstiberium/) and an animation — seeds a neighboring cell at the midpoint of that animation, skipping the source-cell test entirely and using the type named by its [`TiberiumToSpawn`](/keys/tiberiumtospawn/). A Tiberium crate picks a registered type at random, swapping slot 1 for slot 0, and lays a stage-1 patch at the crate plus another ten to twenty scattered around it. A VoxelAnimType declaring [`IsTiberium=yes`](/keys/istiberium/#scope-voxelanimtype) seeds Tiberium at stage 0 where it lands — the ring of eight cells around a meteor's impact, or the single cell beneath any other — always with the type that owns the second Tiberium overlay set. A destroyed object whose type declares [`TiberiumHeal=yes`](/keys/tiberiumheal/) leaves the type in slot 0 at stage 0 through 2 on its own cell and the four beside it.

## Settings the engine parses but never reads

[`TiberiumGrows` in `[MultiplayerDefaults]`](/keys/tiberiumgrows/#scope-global-rules) and [`TiberiumExplosive` in `[SpecialFlags]`](/keys/tiberiumexplosive/#scope-scenarios) are stored and never consulted; the spellings that work are the scenario `[SpecialFlags]` entry and the `[CombatDamage]` entry respectively. [`TiberiumStrength`](/keys/tiberiumstrength/) in `[CombatDamage]` and [`TiberiumTransmogrify`](/keys/tiberiumtransmogrify/) in `[General]` are stored and never consulted at all.
