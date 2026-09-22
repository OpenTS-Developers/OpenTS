---
title: AI base planning and building
summary: "How a computer house plans its base as an ordered build list, picks what to build next, and chooses where to place it."
category: ai-teams
keys:
  - AA
  - AALimit
  - AARatio
  - Adjacent
  - AdvancedPowerPlant
  - AG
  - AIBaseDefenseCoefficient
  - AIBaseDefensePlaceholders
  - AIBaseDefensesWithWalls
  - AIBaseSpacing
  - AIBuildsWalls
  - AIBuildThis
  - AIUseTurbineUpgradeProbability
  - AirstripLimit
  - AirstripRatio
  - AIWallDefense
  - AIWallDefenseCoefficient
  - AIWallTowers
  - BarracksLimit
  - BarracksRatio
  - BaseNormal
  - BaseSizeAdd
  - BuildAA
  - BuildBarracks
  - BuildConst
  - BuildDefense
  - BuildHelipad
  - BuildPDefense
  - BuildPower
  - BuildRadar
  - BuildRefinery
  - BuildTech
  - BuildWeapons
  - ConcreteWalls
  - ConstructionYard
  - DefenseLimit
  - DefenseRatio
  - EWGates
  - GDIBaseDefenseCoefficient
  - GDIPowerPlant
  - GDIPowerTurbine
  - GDIWallDefense
  - GDIWallDefenseCoefficient
  - Helipad
  - HelipadLimit
  - HelipadRatio
  - InfantryBaseMult
  - InfantryReserve
  - IsBaseDefense
  - MaximumBaseDefenseValue
  - MultiplayPassive
  - NodAdvancedPower
  - NodAIBuildsWalls
  - NodBaseDefenseCoefficient
  - NodeCount
  - NodRegularPower
  - NSGates
  - Owner
  - PercentBuilt
  - PlacementDelay
  - PowerEmergency
  - PowersUpBuilding
  - PowerSurplus
  - PowerTurbine
  - Prerequisite
  - PrerequisiteGDIFactory
  - PrerequisiteNodFactory
  - RefineryLimit
  - RefineryRatio
  - RegularPowerPlant
  - TechLevel
  - TeslaLimit
  - TeslaRatio
  - UseMPAIBaseNodes
  - Verses
  - WallTower
  - WarLimit
  - WarRatio
  - Weeder
related:
  - type: system
    id: base-adjacency
---

A computer house builds its base from a **base node list**. Each entry in the list, a **node**, names one BuildingType and the cell where that structure belongs. The house always builds the first node it has not built yet, so list order is build order and there is no priority scoring. The only exceptions are the power and money [interventions](#power-and-money-interventions), which insert a node ahead of the current one.

A node can leave its cell open, and its type can be an instruction instead of a structure:

| Node value | Meaning |
| --- | --- |
| Cell `0,0` | No cell. The placement search picks one when the structure is built. |
| Type `-1` | A base defense placeholder. The planner picks the type and the cell. |
| Type `-2` | Stop. Nothing at or after this node is built. |
| Type `-3` | Plan a perimeter wall. |

## Where the plan comes from

A scenario stores a house's node list in that house's section of the map file. [`NodeCount`](/keys/nodecount/) gives the number of entries, which use the zero-padded keys `000`, `001` and so on, read in order. Each entry is a BuildingType ID followed by the cell X and Y of the structure's north-west foundation corner. An entry that starts with `-` is one of the instructions above. Structures the scenario starts with are ordinary map objects, and a node counts as built when a matching structure already stands on its cell.

A **house following a map plan** is either of these:

- a campaign house, which reads its list from the section named after it, such as `[GDI]`;
- a skirmish or multiplayer house on a map that sets [`UseMPAIBaseNodes=yes`](/keys/usempaibasenodes/). The house holding start position N, counted from zero, reads its list from the [spawn house](/formats/scenario-objects/#spawn-houses) section `[Spawn<N+1>]`.

```ini title="map file"
[GDI]
NodeCount=4
000=MYCONST,42,58 ; example construction yard BuildingType
001=MYWEAP,45,58  ; example war factory BuildingType
002=-1,0,0        ; base defense; the planner picks the type and the cell
003=MYPOWR,0,0    ; example power plant BuildingType; cell picked at build time
```

A house with an empty list generates a plan at either of these moments:

- a computer house's MCV deploys into a [`ConstructionYard=yes`](/keys/constructionyard/) BuildingType outside a campaign game;
- a house passes to the computer while it owns a structure that [`BuildConst`](/keys/buildconst/) lists.

A list the scenario supplies, including one from a spawn house section, prevents generation. At either of those moments, a generated plan puts its first node on the construction yard's cell, and a supplied plan moves only its first `ConstructionYard=yes` node onto the yard.

A house that passes to the computer also has its new plan fitted to the base it already holds. Nodes move onto owned structures of their type, its existing base defenses and power plants are written into the plan, and the walls it owns are sold.

## Building the plan

These steps generate a plan for a house that has none. They never change a list the scenario supplied.

1. **Candidates.** A BuildingType is a candidate when all of these hold:
   - its [`Owner`](/keys/owner/) includes the country this house [acts as](/keys/actslike/);
   - it is [`AIBuildThis=yes`](/keys/aibuildthis/);
   - its [`TechLevel`](/keys/techlevel/) is within the house's tech level;
   - it is not [`Weeder=yes`](/keys/weeder/), unless the map has a veinhole monster;
   - it is not the excluded plug. When the Firestorm addon is enabled, one of the hard-coded IDs `GAPLUG2`, `GAPLUG3` and `GAPLUG4` is picked at random and left out.
2. **Seed.** The queue starts from three rules:
   - A construction yard comes first: the first candidate, in `[BuildingTypes]` order, that [`BuildConst`](/keys/buildconst/) lists.
   - The first [`BuildPower`](/keys/buildpower/) entry the acted country may own comes next, whether or not it is a candidate. When that power plant is also a candidate, step 3 queues it a second time.
   - The first [`BuildBarracks`](/keys/buildbarracks/) entry the country may own moves to the front of the candidate order, and the first [`BuildWeapons`](/keys/buildweapons/) entry to second place, so step 3 tests them first.
3. **Expansion.** The planner makes repeated passes over the remaining candidates. Each pass queues every candidate whose [`Prerequisite`](/keys/prerequisite/) list is met by the queue as it stood when the pass began:
   - A named BuildingType must already be queued. A type that `BuildConst` lists always counts as met.
   - `POWER` always counts as met.
   - `FACTORY` needs the first `BuildWeapons` entry the acted country may own to be queued.
   - `BARRACKS` needs the first `BuildBarracks` entry the country may own to be queued.
   - `RADAR` needs the first [`BuildRadar`](/keys/buildradar/) entry the country may own to be queued.
   - `TECH` needs the first [`BuildTech`](/keys/buildtech/) entry the country may own to be queued.
   - `GDIFACTORY` needs any type from [`PrerequisiteGDIFactory`](/keys/prerequisitegdifactory/) to be queued, and `NODFACTORY` any type from [`PrerequisiteNodFactory`](/keys/prerequisitenodfactory/).

   A [`Helipad=yes`](/keys/helipad/) type is queued one to three extra times. The hard-coded `GAPLUG` waits until a pass queues nothing else. Passes stop when one queues nothing, and a candidate whose prerequisites were never met stays out of the plan.
4. **Refineries.** When the queue holds the first [`BuildRefinery`](/keys/buildrefinery/) entry the acted country may own, `2 - Difficulty` more copies of it are inserted at random positions after it.
5. **Defenses.** A queue of fewer than three entries becomes the plan unchanged, with no defenses and no wall node from step 6. Otherwise the first three entries are copied as they stand. Before copying each later entry, the plan adds base-defense placeholders:
   - The wanted count is `(C - 2000) / 1500`, truncated to a whole number, times the acted side's [`AIBaseDefenseCoefficient`](/keys/aibasedefensecoefficient/), truncated again. `C` is the combined cost, at this house's prices, of the queue entries copied so far, not counting the first. Towers and placeholders add nothing to `C`.
   - When the wanted count exceeds the number of placeholders added so far, the difference is added as `-1` placeholders. Each is preceded by the first [`AIWallTowers`](/keys/aiwalltowers/) entry the acted country may own, when there is one.

   For example, with a coefficient of 1, two placeholders are wanted once the entries copied after the first cost 5000 in total. Below 3500, none are wanted.

   After the last entry, `(3 - Difficulty) * `[`AIBaseDefensePlaceholders`](/keys/aibasedefenseplaceholders/) further placeholders follow, each preceded by that tower, in either of these cases:
   - the side will not build a wall, because the side's [`AIBuildsWalls`](/keys/aibuildswalls/#scope-side) or the global [`AIBuildsWalls`](/keys/aibuildswalls/#scope-global-rules) is `no`;
   - the side sets [`AIBaseDefensesWithWalls=yes`](/keys/aibasedefenseswithwalls/).
6. **Wall.** A `-3` node closes the list when both the global `AIBuildsWalls` and the side's are `yes`.

`Difficulty` in steps 4 and 5 is the house's [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot): `[Easy]` is 0, `[Normal]` 1 and `[Difficult]` 2. A computer house normally holds the inverse of the setting the player chose, so a harder game gives the plan more extra refineries and more placeholders. The table assumes that inverse. In a game with more than one human player, [`CompEasyBonus`](/keys/compeasybonus/) can move a computer house [one slot lower](/systems/difficulty/#the-computers-bonus-with-more-than-one-human), and a launch file's [`[HouseHandicaps]`](/formats/spawn-ini/) entry sets a seat's slot directly.

| Setting chosen | Slot the computer house holds | `2 - Difficulty`, the extra refineries | `3 - Difficulty`, the placeholder multiplier |
| --- | --- | --- | --- |
| Easy | 2, the `[Difficult]` section | 0 | 1 |
| Normal | 1, the `[Normal]` section | 1 | 2 |
| Hard | 0, the `[Easy]` section | 2 | 3 |

The same `3 - Difficulty` term caps the [wall defenses](#walls-and-gates) a house appends after its perimeter wall, when its side lists a wall tower its country may own.

## Choosing what to build next

A computer house picks its next structure when all of these hold:

- it is not a human player's house;
- its country is not [`MultiplayPassive=yes`](/keys/multiplaypassive/);
- its production mode allows structure work, which is always the case in a campaign;
- it has no structure already pending;
- it owns a structure that [`BuildConst`](/keys/buildconst/) lists.

It takes the first node that does not count as built. A node counts as built when any of these holds:

- a structure of the node's type, owned by this house, has its foundation corner on the node's cell;
- the house's structure on that cell already carries the node's type as an upgrade. Each node that names the same upgrade at that cell needs its own copy;
- the node names a wall type, and its cell holds that wall's overlay or any structure.

A node whose cell is `0,0` never matches a structure, so it stays unbuilt until its structure is placed and the placement writes a cell into it.

What happens to the node depends on its type:

- A `-2` node stops the house. Nothing after it is built.
- A `-3` node is removed from the list, and the [perimeter wall planner](#walls-and-gates) appends the wall nodes.
- A `-1` placeholder, or a node of one of the acted side's [`AIWallTowers`](/keys/aiwalltowers/) types that has no cell, goes to the [defense planner](#base-defenses). If the planner cannot fill it, the node is deleted. A tower node also takes the node after it.
- Any other node becomes the house's pending structure. A construction yard starts producing it once the house is [started](/keys/production/) and has more than 10 credits.

Because the house always takes the first unbuilt node, a node that no owned factory can produce holds up every node after it.

## Choosing a spot

The house picks the cell when the finished structure leaves the construction yard. It looks for the first unbuilt node of the structure's type:

- If that node has a cell, the structure goes there when it is an upgrade or when the [compactness test](#the-compactness-test) accepts the cell. Otherwise the placement search picks a new cell.
- If the node has no cell, or no node matches, the placement search picks one. An upgrade instead goes onto an owned structure it can plug into, preferring the least upgraded one.

The chosen cell is written back into the node.

An upgrade node that already has a cell keeps it only while the structure on that cell is the type it [plugs into](/keys/powersupbuilding/) and still has room for it. Otherwise the node loses its cell before production starts. This check applies only to an upgrade whose [`PowersUpToLevel`](/keys/powersuptolevel/) is `-1`.

### The placement search

Every structure placed on the map, by any house, reserves its footprint plus [`AIBaseSpacing`](/keys/aibasespacing/) cells on every side for its owner. The exception is a structure that sets [`UndeploysInto`](/keys/undeploysinto/) and is not a construction yard, which reserves nothing. The owner's **base rectangle** grows to hold that reserved area.

The search starts from the edge of the reserved area: cells with at least one, but not all eight, reserved neighbors. It tries those edge cells in order of preference. An ordinary structure prefers cells near the **base center**. A base defense prefers cells that existing defenses cover least, toward the quadrant chosen for it in [Base defenses](#base-defenses).

The base center is a cell set by one of three events:

- a computer house's MCV deploys outside a campaign game, which sets it to the new construction yard's cell;
- a house passes to the computer, which sets it to the cell of a construction yard it owns;
- the [Auto Base Building](/mapping/actions/taction-base-building/) trigger action turns base building on, which sets it to the house's first construction yard.

A house with no base center skips the search and all of its tests. Its structure targets the center of the house's structures, averaged with more weight on expensive ones. The defense quadrants and the wall height reference are then measured from cell `0,0`. A campaign plan with `0,0` cells or `-1` placeholders therefore needs the Auto Base Building action.

From each edge cell the search looks outward, away from the reserved area, and tries up to three positions in a line. A position is accepted when all of these hold:

- the structure's footprint, grown by `AIBaseSpacing` on every side, contains no cell this house has reserved and lies inside the playable area;
- every cell of the foundation is clear to build on. A type that lays a tile under itself needs only one clear cell, and a [`PlaceAnywhere=yes`](/keys/placeanywhere/) type skips this test;
- the ground height at the position is within 2 of the height at the base center;
- the compactness test accepts the position.

When nothing passes, the search relaxes the first test in two stages, so a cramped base still builds. Each edge cell gets a second try that starts one cell closer to the base and drops the `AIBaseSpacing` margin from that test. After every edge cell has failed, a second pass skips that test entirely. If the second pass also fails, the search finds no cell, and the structure is abandoned as described below.

### The compactness test

The compactness test always passes for a house following a map plan, so its base spreads wherever the other tests allow.

For every other house, the test requires a cell this house has reserved near the candidate position. The area checked is the footprint grown by `AIBaseSpacing`, plus a ring outside it. The ring is one cell wide on the north and west sides and `AIBaseSpacing` plus one cells wide on the south and east. This keeps a skirmish or multiplayer base joined to what it already holds.

### Clearing the site

Allied vehicles, infantry and aircraft standing on the foundation are told to move off it. The construction yard then waits [`PlacementDelay`](/keys/placementdelay/) minutes before trying again.

The structure is abandoned when the search finds no cell, when placement fails, or when the site holds a permanent obstruction. A permanent obstruction is any of these:

- an overlay that cannot be built over, except a GDI concrete wall under a [`WallTower`](/keys/walltower/);
- a terrain object;
- a structure that cannot take this one as an upgrade;
- an object of a house that is not an ally.

Abandoning a structure refunds its cost and deletes it. A wall or gate node is then deleted. For any other type, every node claiming the failed cell loses its cell, so the next attempt searches again. An upgrade with no structure to go on is abandoned too, and its node is deleted when it is the next unbuilt node.

After a `WallTower` is placed, the next node after it whose type is [`IsBaseDefense=yes`](/keys/isbasedefense/) moves onto the tower's cell.

:::caution[Adjacency does not limit computer placement]
[`Adjacent`](/keys/adjacent/) and [`BaseNormal`](/keys/basenormal/) govern player-controlled placement only. The computer's search uses the tests above and never checks [base adjacency](/systems/base-adjacency/). Raising `Adjacent` does not loosen a computer base, and `BaseNormal=no` does not stop the computer building next to a structure.
:::

## Base defenses

A **threat cell** is a cell the base is expected to be attacked through, and the house's list of them is its **threat ring**. Only [the perimeter wall planner](#walls-and-gates) fills a threat ring, with the wall cells it lays. It does so only for a side whose [`AIWallTowers`](/keys/aiwalltowers/) names a type the acted country may own. Each defense node filled from the ring uses up one of its cells. A house with no ring, or an empty one, fills its defense nodes through the placement search instead.

The defense planner first picks the quadrant of the base, around the base center, that most needs a defense. It totals the anti-air, anti-armor and anti-infantry values of the house's structures in each quadrant and takes the quadrant with the lowest total. With a threat ring, only quadrants holding at least one threat cell are considered.

The planner then picks the defense category with the smallest share of that quadrant's total. Ties go to anti-infantry, then anti-armor, then anti-air.

A BuildingType is a candidate for that category when all of these hold:

- the country the house acts as may own it;
- its value in that category is above zero;
- its `TechLevel` is within the house's tech level;
- its prerequisites are met by the house's owned structures other than base defenses and wall towers, plus every `AIWallTowers` entry of the acted side. As in planning, `POWER` and any type `BuildConst` lists always count as met.

When the chosen category has no candidate, the planner uses the first of anti-armor, anti-infantry and anti-air that has one. With no candidates in any category, the node is deleted. The planner draws one candidate at random, weighted by `10000 / cost` plus its value in the category, so cheap defenses are favored.

Each existing defense covers the cells within 6 of it. Its coverage of a cell is its value in the category, weakened with distance as `value / ((distance - 1) * 0.1 + 1)`.

The defense's cell depends on the threat ring:

- With a threat ring, the defense takes the threat cell in the chosen quadrant that existing defenses cover least in that category.
- Without a ring, the placement search picks the cell, and a failed search deletes the node.

A **tower node** is a node of one of the acted side's `AIWallTowers` types that has no cell. It is filled differently from a `-1` placeholder:

- The planner chooses only among defenses that [plug into](/keys/powersupbuilding/) that tower type.
- The tower node keeps its type and takes the chosen cell.
- If the next node is still a `-1` placeholder, it becomes the chosen defense at the same cell. Otherwise the chosen defense is not queued.
- When that defense is queued and the house has a threat ring, a wall node at the same cell is deleted.

When no candidate plugs into the tower, the tower node becomes a standalone defense, as a `-1` placeholder would. The placeholder after it is filled later.

### Defense values

A BuildingType's anti-air, anti-armor and anti-infantry values are computed when the rules are loaded. Only a type with [`IsBaseDefense=yes`](/keys/isbasedefense/) and a primary weapon gets them. From that weapon, `damage` is `Damage / (ROF * 0.025)`, truncated to a whole number:

- A projectile with [`AA=yes`](/keys/aa/) sets the anti-air value to `damage` times the warhead's [`Verses`](/keys/verses/) percentage against `heavy` armor.
- A projectile with [`AG=yes`](/keys/ag/), the default, sets the anti-armor value the same way, and the anti-infantry value from the `Verses` percentage against `none`.

Each value is capped at [`MaximumBaseDefenseValue`](/keys/maximumbasedefensevalue/). Every other type keeps all three values at zero, so it is never a defense candidate.

An owned structure whose type has a zero value in a category counts the first non-zero value among the upgrades plugged into it.

## Walls and gates

The perimeter wall planner walks the four edges of the **wall ring**, the base rectangle grown by one cell on each side. A cell can take a wall when all of these hold:

- its height is within 2 of the height at the base center;
- neither it nor its outward neighbor is rock, water or ice;
- it has no overlay, or only one that can be built over;
- neither it nor its outward neighbor holds a structure;
- neither it nor its outward neighbor holds a terrain object;
- it is not a ramp;
- it lies inside the playable area.

Neighboring suitable cells along an edge form a run. What a run becomes depends on the cell that ends it:

- A cell that fails only the overlay or ramp test makes the run plain wall at any length, with no gate.
- After such a cell, the next cell on the same edge that fails any other test is treated the same way. The run it ends also becomes plain wall with no gate, even at five or more cells.
- Any other end, including the end of the edge, gives a run of five or more cells wall nodes with a gate at its midpoint. A shorter run is left without a wall.

Walls use the first [`ConcreteWalls`](/keys/concretewalls/) entry the acted country may own. A gate takes three cells of its run: the first [`EWGates`](/keys/ewgates/) entry on the north and south edges, and the first [`NSGates`](/keys/nsgates/) entry on the east and west. All wall nodes are appended before any gate node.

Make sure every country whose side builds walls may own a `ConcreteWalls`, an `EWGates` and an `NSGates` entry. The game crashes when the planner lays a wall or gate and the country may own no entry of that kind.

For a side with an [`AIWallTowers`](/keys/aiwalltowers/) entry the acted country may own, the wall cells also become the [threat ring](#base-defenses). The planner then appends pairs of a tower node and a `-1` placeholder. It adds one pair per five wall nodes, capped at `(3 - Difficulty) * `[`AIWallDefenseCoefficient`](/keys/aiwalldefensecoefficient/)` + `[`AIWallDefense`](/keys/aiwalldefense/), rounded down to a whole number of pairs.

Finally the base rectangle becomes the wall ring, so a later `-3` node plans its wall one ring further out.

:::note[Side behavior comes from the side a house acts as]
Every side-specific choice on this page is read from the side of the country the house [acts as](/keys/actslike/), through the section named after that side. The first two sides start from the `GDI`- and `Nod`-prefixed keys in the rules. A house acting for no country, or for a country in no side, plans with the defaults of a third or later side. Those are a coefficient of 1, two placeholders per difficulty step, no towers, and a wall when the global `AIBuildsWalls` allows one.
:::

## Power and money interventions

A house that is not following a map plan checks power before it starts a structure. It inserts a power plant node just before the current node when all of these hold:

- the node's drain added to the house's current drain exceeds its current power output;
- the node is not a [`BuildConst`](/keys/buildconst/) construction yard;
- the node's type drains power.

The plant comes from the acted side. The house takes the first of these that applies:

1. The side's [`PowerTurbine`](/keys/powerturbine/), when the house owns one of the side's [`RegularPowerPlant`](/keys/regularpowerplant/) structures with a free upgrade slot and a random draw passes [`AIUseTurbineUpgradeProbability`](/keys/aiuseturbineupgradeprobability/). That probability is a fraction of 1. At its default, the turbine is chosen whenever a slot is free.
2. The side's [`AdvancedPowerPlant`](/keys/advancedpowerplant/), when the structures the house owns meet its prerequisites.
3. The side's `RegularPowerPlant`.
4. When the side names no `RegularPowerPlant`, the first [`BuildPower`](/keys/buildpower/) entry the acted country may own.

With no plant to insert, the node is built as it stands.

A house that is not following a map plan also sells its base when it cannot make money. That happens when it lacks a refinery or a harvester and cannot afford the missing one. It works through the node list from the back:

- A structure carrying an upgrade loses its top upgrade, and the house is credited that upgrade's full cost.
- Any other structure is sold.
- Walls and gates are sold and their nodes removed.

The house sells until its money covers what it needs: a harvester when it owns both a refinery and a war factory, a refinery otherwise. It then cancels everything in production. It either orders the harvester, or inserts a refinery node ahead of the next unbuilt node and removes the later refinery nodes.

The house sends every vehicle, infantry and aircraft it owns to hunt in any of these cases:

- selling everything in the list does not raise enough;
- the money is raised only when selling reaches the first node in the list. The house still orders the harvester or inserts the refinery node;
- it needs a refinery and the next unbuilt node is the first in the list, so no refinery node is inserted.

## Rebuilding

Rebuilding needs no separate mechanism. A structure taken off the map, destroyed or sold, no longer matches its node, so the node becomes a gap the house fills again in list order.

When a structure that matches a node is taken off the map, every other node on its cell loses its cell. What happens to a base defense's node depends on the house:

- A house that is not following a map plan turns an `IsBaseDefense=yes` node back into a `-1` placeholder, so the planner picks a fresh type and cell.
- A house following a map plan keeps the node and rebuilds the same defense on the same cell.

## Parsed settings without effect

`[AI]` holds a block of ratio and limit settings that no decision reads: [`RefineryRatio`](/keys/refineryratio/), [`RefineryLimit`](/keys/refinerylimit/), [`BarracksRatio`](/keys/barracksratio/), [`BarracksLimit`](/keys/barrackslimit/), [`WarRatio`](/keys/warratio/), [`WarLimit`](/keys/warlimit/), [`DefenseRatio`](/keys/defenseratio/), [`DefenseLimit`](/keys/defenselimit/), [`AARatio`](/keys/aaratio/), [`AALimit`](/keys/aalimit/), [`TeslaRatio`](/keys/teslaratio/), [`TeslaLimit`](/keys/teslalimit/), [`HelipadRatio`](/keys/helipadratio/), [`HelipadLimit`](/keys/helipadlimit/), [`AirstripRatio`](/keys/airstripratio/), [`AirstripLimit`](/keys/airstriplimit/), [`BaseSizeAdd`](/keys/basesizeadd/), [`InfantryReserve`](/keys/infantryreserve/), [`InfantryBaseMult`](/keys/infantrybasemult/) and [`PowerEmergency`](/keys/poweremergency/).

The type lists [`BuildDefense`](/keys/builddefense/), [`BuildPDefense`](/keys/buildpdefense/), [`BuildAA`](/keys/buildaa/) and [`BuildHelipad`](/keys/buildhelipad/) are parsed but never read. Base defenses come from the [defense values](#defense-values) instead.

[`PowerSurplus`](/keys/powersurplus/) is parsed but never reaches a house, whose power margin stays at zero. The power intervention therefore fires as soon as a structure's drain would exceed the house's output.

[`PercentBuilt`](/keys/percentbuilt/) in a house's map section is read and written back unchanged, and controls nothing.
