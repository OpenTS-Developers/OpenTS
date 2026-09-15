---
title: AI base planning and building
summary: "Lays a computer house's base out as an ordered node list, then resolves what to build next and where to put it."
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

A computer house does not weigh up what to build next. It works down a **base node list**, an ordered list in which each entry, a **node**, names one BuildingType and the cell that structure belongs on. Position in the list is the build order, and nothing else bears on it: there is no priority scoring, and the house always takes the first node it has not built yet.

Not every node names a structure at a place. A cell of `0,0` names no cell, and hands the spot to the placement search instead. Three of the values that would otherwise name a BuildingType are instructions to the planner. `-1` asks for a base defense whose type the planner chooses, `-2` stops the scan there, and `-3` runs the perimeter wall planner.

## Where the plan comes from

The list is stored in the house's own map section, not in a section of its own. [`NodeCount`](/keys/nodecount/) gives the entry count, and entries are read in order from zero-padded three-digit keys. The first field names an ObjectType ID and the next two are the cell X and Y, the north-west corner of the structure's foundation. A value beginning with `-` is one of the three instructions instead. Structures a scenario starts with are ordinary map objects, which the node list then matches.

A **house following a map plan** is a campaign house, or a skirmish or multiplayer house on a map that sets [`UseMPAIBaseNodes=yes`](/keys/usempaibasenodes/). A campaign house reads its list from its own section, `[GDI]` or `[Nod]`. On such a map the house holding start position N, counted from zero, reads its list from the [spawn house](/formats/scenario-objects/#spawn-houses) section `[Spawn<N+1>]`.

```ini title="map file"
[GDI]
NodeCount=4
000=MYCONST,42,58 ; example construction yard BuildingType
001=MYWEAP,45,58  ; example war factory BuildingType
002=-1,0,0        ; base defense; the planner picks the type and the cell
003=MYPOWR,0,0    ; example power plant BuildingType; cell picked at build time
```

A house whose list is empty generates one in either of two moments. One is when an MCV of a non-human house deploys into a [`ConstructionYard=yes`](/keys/constructionyard/) BuildingType outside a campaign game, and the other is when a house passes to the computer. A scenario-supplied list, a spawn house section's included, suppresses generation entirely. A generated plan has its first node placed on the construction yard's cell, while a supplied plan keeps its cells and only its first `ConstructionYard=yes` node moves onto the yard.

## Building the plan

The six steps below generate a plan for a house that has none. A list a scenario supplies is taken as it stands, so none of them runs over it.

1. **Candidates.** A BuildingType is a candidate while all of this holds:
   - its [`Owner`](/keys/owner/) includes the country this house [acts as](/keys/actslike/);
   - it is [`AIBuildThis=yes`](/keys/aibuildthis/);
   - its [`TechLevel`](/keys/techlevel/) is within the house's scenario tech level;
   - it is not [`Weeder=yes`](/keys/weeder/), or the map has a veinhole monster;
   - it is not the excluded plug. Under the Firestorm addon the planner draws one of the hard-coded IDs `GAPLUG2`, `GAPLUG3` and `GAPLUG4` at random and leaves that type out.
2. **Seed.** The first [`BuildConst`](/keys/buildconst/) entry that passes that filter, then the first [`BuildPower`](/keys/buildpower/) entry that country may own, when there is one. The first such [`BuildBarracks`](/keys/buildbarracks/) entry moves to the head of the candidate list and the first such [`BuildWeapons`](/keys/buildweapons/) entry to second place.
3. **Expansion.** Repeated passes append every candidate whose [`Prerequisite`](/keys/prerequisite/) list the queue already satisfies. A generic prerequisite resolves through `BuildWeapons`, `BuildBarracks`, [`BuildRadar`](/keys/buildradar/) or [`BuildTech`](/keys/buildtech/). A `GDIFACTORY` group resolves through its [`PrerequisiteGDIFactory`](/keys/prerequisitegdifactory/) list and a `NODFACTORY` group through its [`PrerequisiteNodFactory`](/keys/prerequisitenodfactory/) list, by any of those types already queued, and a `BuildConst` construction yard always counts as satisfied. A [`Helipad=yes`](/keys/helipad/) type is appended one to three extra times. The hard-coded `GAPLUG` waits for a pass that adds nothing else.
4. **Refineries.** `2 - Difficulty` extra copies of the first [`BuildRefinery`](/keys/buildrefinery/) entry that country may own, at random positions after the first refinery.
5. **Defenses.** A queue shorter than three entries (a country with fewer than three AI-buildable structures) is written to the plan as it stands, with nothing woven in. Otherwise the plan works through the queue from its fourth entry with a running build cost, which starts as the combined cost of the second and third entries. For each entry it first computes `(cost - 2000) / 1500` from the cost as it stands, truncates that to a whole number, and scales it by the acted side's [`AIBaseDefenseCoefficient`](/keys/aibasedefensecoefficient/), then truncates the product again. A count at or below zero asks for none. It then appends the entry and adds its cost to the running total. Each unit of shortfall becomes a `-1` placeholder, preceded by the first [`AIWallTowers`](/keys/aiwalltowers/) entry the acted country may own when there is one. When the side will not build a wall (its own [`AIBuildsWalls`](/keys/aibuildswalls/#scope-side) or the global [`AIBuildsWalls`](/keys/aibuildswalls/#scope-global-rules) is `no`), or when its [`AIBaseDefensesWithWalls`](/keys/aibasedefenseswithwalls/) is `yes`, `(3 - Difficulty) * `[`AIBaseDefensePlaceholders`](/keys/aibasedefenseplaceholders/) further placeholders follow, each preceded by that tower.
6. **Wall.** A `-3` node closes the list when both the global `AIBuildsWalls` and the side's own are `yes`.

`Difficulty` in steps 4 and 5 is the house's own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot): `[Easy]` is 0, `[Normal]` 1 and `[Difficult]` 2. A computer house is handed the inverse of the setting the player chose. The table works both terms out for each setting. Read the two right-hand columns downward: the harder the player set the game, the more extra refineries the plan holds and the larger the placeholder counts in step 5 come out.

| Setting chosen | Slot the computer house holds | `2 - Difficulty`, the extra refineries | `3 - Difficulty`, the term in the placeholder counts |
| --- | --- | --- | --- |
| Easy | 2, the `[Difficult]` section | 0 | 1 |
| Normal | 1, the `[Normal]` section | 1 | 2 |
| Hard | 0, the `[Easy]` section | 2 | 3 |

Where a house's side lists a wall tower its country may own, the same `3 - Difficulty` term caps the [wall defenses](#walls-and-gates) it appends after its perimeter wall.

## Choosing what to build next

A house takes the first node not counted as built while all of this holds:

- it is not a human player's house;
- its country is not [`MultiplayPassive=yes`](/keys/multiplaypassive/);
- its production mode allows structure work;
- it has no structure already committed;
- it owns a construction yard.

A node counts as built under any of these:

- a building of the node's own type, owned by this house, stands on the node's cell;
- the building on the node's cell is this house's and takes the node's type as an upgrade;
- the node names a wall type and its cell has that wall's overlay;
- the node names a wall type and its cell has any building.

A node whose cell is `0,0` names no cell, so no building matches it. A `-3` node deletes itself before running the wall planner. When the defense planner fails on a `-1` node or on a cell-less node of the acted side's [`AIWallTowers`](/keys/aiwalltowers/) entry, that node is deleted, and a tower node takes the following node with it. Any other node becomes the house's pending structure. Because the first unbuilt node is always the one taken, a node no owned factory can produce holds up every node behind it.

## Choosing a spot

Placement is resolved when the finished structure leaves the construction yard. A real cell on the matching node is used when the type is an upgrade or the compactness test accepts that cell. Otherwise a search runs, and its result is written back into the node.

Placing any structure flags its footprint, expanded by [`AIBaseSpacing`](/keys/aibasespacing/) on all sides, as occupied by its house, and grows that house's base rectangle to contain the footprint. The search ranks the frontier of that area: cells with at least one, but not all eight, occupied neighbors. An ordinary structure ranks them by distance from the base center, and a base defense ranks them by how thinly the cell is already covered. From each ranked cell the search steps outward, away from the mass of the base, clear of the structure's own footprint plus `AIBaseSpacing`. A step is accepted while all of this holds:

- the step's footprint rectangle, grown by that spacing, holds no cell this house already occupies;
- that same rectangle lies inside the playable area;
- every cell of the structure's own foundation is clear to build on, except that a type laying its own tile underneath itself needs only one clear cell;
- the ground height at the step is within 2 of the height under the base center;
- the compactness test accepts the step.

The first two terms are settled together as one test, and that test is skipped altogether on a second pass over the ranked cells, so a cramped base still builds. A search that accepts nothing returns cell `0,0`.

That compactness test passes unconditionally for a house following a map plan, so its base spreads wherever the other tests allow. Every other house requires an already-occupied cell inside the candidate footprint padded by `AIBaseSpacing`, or in the ring just outside that margin: one cell out on the north and west, and `AIBaseSpacing` plus one on the south and east. That requirement welds a skirmish or multiplayer base to what it already holds.

An allied vehicle, infantry or aircraft standing in the placement zone is ordered to move, and the factory then waits [`PlacementDelay`](/keys/placementdelay/) minutes. Two things abandon the structure: a permanent obstruction, and a failed placement. A permanent obstruction is an overlay, a terrain object, a building that cannot take the structure as an upgrade, or another house's object. Either one refunds the cost already paid, deletes the object under construction, and clears the pending structure. The node is deleted when its type is a wall or a gate; otherwise every node claiming that cell has its cell reset to `0,0`. After a `WallTower` is placed, the next base-defense node moves onto the tower's cell.

:::caution[The computer does not run the adjacency proximity check]
[`Adjacent`](/keys/adjacent/) and [`BaseNormal`](/keys/basenormal/) govern player-controlled placement only. The computer's search tests foundation cells, height difference, its own reservation footprint and the compactness test, and never reads [base adjacency](/systems/base-adjacency/). Raising `Adjacent` does not loosen a computer base, and `BaseNormal=no` does not stop the computer building beside a structure.
:::

## Base defenses

The planner is handed a list of cells the base is expected to be attacked through. Each one is a **threat cell**, and the list as a whole is the house's **threat ring**. Exactly one thing ever fills it: [the perimeter wall planner](#walls-and-gates) records the wall cells it laid, and only for a side whose [`AIWallTowers`](/keys/aiwalltowers/) names a type the acted country may own. Every other house reaches the defense planner with no threat ring at all, and the qualifications below all turn on which of those two cases applies.

A defense node is filled in against the quadrant of the base that needs it most. Each owned building's anti-air, anti-armor and anti-infantry values are summed per quadrant. Each of the three sums is stamped into a per-cell coverage map out to radius 6, falling off as `value / ((distance - 1) * 0.1 + 1)`. The quadrant with the lowest combined total wins, restricted to quadrants holding at least one threat cell where a ring was supplied. The category chosen is the one least represented there: the planner measures each against a predicted enemy composition of a fixed 0.33 and takes the largest shortfall. Ties resolve to anti-infantry first, then anti-armor over anti-air.

A BuildingType is a candidate for that category while all of this holds:

- the country the house acts as may own it;
- its value in that category is above zero;
- its `TechLevel` is within the house's reach;
- its prerequisites are met by the non-defense buildings the house owns, plus the acted side's `AIWallTowers`.

An empty list falls back to anti-armor, then anti-infantry, then anti-air, and all three empty deletes the node. One candidate is drawn at random, weighted by `10000 / cost + its value in that category`, so cheap defenses dominate. It consumes the best-scoring cell of the threat ring where one was supplied, and takes the placement search's result otherwise.

What the node then receives depends on which kind of node it is. A `-1` placeholder takes both the chosen type and the chosen cell. A tower node chooses among the defenses that [plug into](/keys/powersupbuilding/) that tower. It keeps its own type and takes only the cell, and the chosen upgrade is written into the node after it at that same cell, but only while the following node is still a `-1` placeholder. When the upgrade is written there, a wall node already claiming the cell is deleted as well, and only when a threat ring was supplied. A tower none of the country's defenses plug into is dropped: the node takes a standalone defense as a placeholder would, and the placeholder after it waits for the next pass.

A BuildingType's three category values are computed from rules once the weapons are loaded, and only for a type with [`IsBaseDefense=yes`](/keys/isbasedefense/). From its primary weapon, `damage` is `Damage / (ROF * 0.025)` truncated to a whole number. A projectile with [`AA=yes`](/keys/aa/) sets `AntiAirValue` to `damage` multiplied by the warhead's [`Verses`](/keys/verses/) percentage against `heavy` armor. A projectile with [`AG=yes`](/keys/ag/), the default, sets `AntiArmorValue` from that same `heavy` figure, and `AntiInfantryValue` from the `Verses` percentage against `none`. All three are capped at [`MaximumBaseDefenseValue`](/keys/maximumbasedefensevalue/). A type without `IsBaseDefense=yes`, or with no primary weapon, keeps all three at zero and never enters a candidate list. At runtime, a building whose own value is zero reports the first non-zero value among its plugged-in upgrades instead.

## Walls and gates

The wall ring is the base rectangle grown by one cell on each side, walked along its four edges. A cell takes a wall while all of this holds:

- its height is within 2 of the height under the base center;
- neither it nor its outward neighbor is rock, water or ice;
- it has no overlay;
- neither it nor its outward neighbor holds a building;
- neither it nor its outward neighbor holds a terrain object;
- its ramp is flat;
- it lies inside the playable area.

A run becomes wall nodes once it reaches five cells, or sooner when an overlay or a ramp cuts it short.

Wall nodes come from the first [`ConcreteWalls`](/keys/concretewalls/) entry the acted country may own, and all of them are appended before any gate node. A gate node takes the midpoint of a run and consumes three wall slots: [`EWGates`](/keys/ewgates/) on the north and south edges and [`NSGates`](/keys/nsgates/) on the east and west. A run cut short by an overlay or a ramp is laid as plain wall. For a side with an [`AIWallTowers`](/keys/aiwalltowers/) entry the acted country may own, the wall cells also become the [threat ring](#base-defenses) the defense planner draws from. Pairs of that tower's node and a `-1` node are then appended, `0.2` per wall node, capped at `(3 - Difficulty) * `[`AIWallDefenseCoefficient`](/keys/aiwalldefensecoefficient/)` + `[`AIWallDefense`](/keys/aiwalldefense/)`, and truncated to a whole number of pairs. The base rectangle then becomes the wall ring, so the next wall is planned one ring further out.

:::note[Side behavior comes from the side a house acts as]
Every side-specific choice above is read from the side of the country the house [acts as](/keys/actslike/), through the section matching that side's own name. The first two sides start from the `GDI`- and `Nod`-prefixed keys in the rules, so the shipped rules build the bases they always did. A house acting for no country, or for a country belonging to no side, plans with the defaults a third side starts with: a coefficient of 1, two placeholders per difficulty step, no towers, and a wall.
:::

## Power and money interventions

A power plant node is inserted immediately before the node the house is about to build while all of this holds:

- the house is not following a map plan;
- the node's own drain added to the house's current drain exceeds its current power output;
- the node is not a [`BuildConst`](/keys/buildconst/) construction yard;
- the node's type draws power at all.

Which plant goes in depends on the acted side, and the choices are tried in order. The side's [`PowerTurbine`](/keys/powerturbine/) goes in first. It is taken when the house owns one of the side's [`RegularPowerPlant`](/keys/regularpowerplant/) that has a free upgrade slot, and a random draw falls under [`AIUseTurbineUpgradeProbability`](/keys/aiuseturbineupgradeprobability/). That is a fraction of 1 that defaults to 1, so the turbine is taken whenever a slot is free unless the value is lowered. Next comes the side's [`AdvancedPowerPlant`](/keys/advancedpowerplant/) when the buildings the house owns meet that type's prerequisites, then its `RegularPowerPlant`. When the side names no plant at all, the first [`BuildPower`](/keys/buildpower/) entry the acted country may own goes in. With nothing to insert, the node is built as it stands.

A house that cannot make money, again only when it is not following a map plan, sells its base from the back of the node list forward. It sells until the proceeds cover a harvester, where it owns both a refinery and a war factory, or a refinery otherwise. It then abandons its factories and either orders that harvester or inserts a refinery node at the current build position. Selling out the whole list without raising enough sends every unit it owns to hunt.

## Rebuilding

Rebuilding needs no separate mechanism: a destroyed structure stops matching its node, and the node becomes the next hole in the list. When a building is taken off the map, every other node claiming its cell has its cell cleared. A house that is not following a map plan retires an `IsBaseDefense=yes` node to a `-1` placeholder, so the planner picks a fresh type and cell for it. A house following a map plan keeps the node and rebuilds the same defense on the same cell.

## Parsed settings without effect

`[AI]` holds a block of ratio and limit settings that no decision reads: [`RefineryRatio`](/keys/refineryratio/), [`RefineryLimit`](/keys/refinerylimit/), [`BarracksRatio`](/keys/barracksratio/), [`BarracksLimit`](/keys/barrackslimit/), [`WarRatio`](/keys/warratio/), [`WarLimit`](/keys/warlimit/), [`DefenseRatio`](/keys/defenseratio/), [`DefenseLimit`](/keys/defenselimit/), [`AARatio`](/keys/aaratio/), [`AALimit`](/keys/aalimit/), [`TeslaRatio`](/keys/teslaratio/), [`TeslaLimit`](/keys/teslalimit/), [`HelipadRatio`](/keys/helipadratio/), [`HelipadLimit`](/keys/helipadlimit/), [`AirstripRatio`](/keys/airstripratio/), [`AirstripLimit`](/keys/airstriplimit/), [`BaseSizeAdd`](/keys/basesizeadd/), [`InfantryReserve`](/keys/infantryreserve/), [`InfantryBaseMult`](/keys/infantrybasemult/) and [`PowerEmergency`](/keys/poweremergency/). The type lists [`BuildDefense`](/keys/builddefense/), [`BuildPDefense`](/keys/buildpdefense/), [`BuildAA`](/keys/buildaa/) and [`BuildHelipad`](/keys/buildhelipad/) are parsed but never read; base defenses come from the computed values above, not from a list. [`PowerSurplus`](/keys/powersurplus/) is parsed into the rules but never reaches a house, whose own power margin stays at zero, so the power interjection fires on the first shortfall rather than on a configured cushion. [`PercentBuilt`](/keys/percentbuilt/) in a house's map section is read and written back unchanged and controls nothing.
