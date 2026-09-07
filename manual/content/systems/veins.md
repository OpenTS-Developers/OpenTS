---
title: Veins and veinhole monsters
summary: "Veinhole monsters grow fields of vein overlay that hurt what stands in them and fill a house's weed pool."
category: buildings-economy
keys:
  - Dock
  - DockUnload
  - HarvesterDumpRate
  - HarvesterLoadRate
  - ImmuneToVeins
  - IsVeinhole
  - IsVeinholeMonster
  - IsVeins
  - MaxVeinholeGrowth
  - Storage
  - TiberiumFarScan
  - TiberiumNearScan
  - VeinAttack
  - VeinDamage
  - VeinGrowthEnabled
  - VeinGrowthRate
  - Veinhole
  - VeinholeGrowthRate
  - VeinholeMonsters
  - VeinholeMonsterStrength
  - VeinholeShrinkRate
  - VeinholeTypeClass
  - VeinholeWarhead
  - WeedCapacity
  - Weeder
related:
  - type: action
    id: TACTION_VEIN_GROWTH
  - type: enum
    id: LandType
  - type: system
    id: tiberium
---

## Veinhole monsters

This section introduces the entity the rest of the page turns on. Anyone already familiar with veinhole monsters and how one is declared can skip to [vein fields](#vein-fields).

A veinhole monster stands in one cell and owns the field of veins that spreads out from it. It is neither a building nor a vehicle: it belongs to no house, and every monster in the scenario is given its logic and drawn from a list of its own rather than through the ordinary object layers. Its type definition is the TerrainType named by [`VeinholeTypeClass`](/keys/veinholetypeclass/), which supplies the monster's maximum strength, armor, and immunity.

```ini title="rules.ini"
[General]
VeinholeTypeClass=VEINTREE

[VEINTREE]
Name=Veinhole Tree
Image=None
Armor=None
Strength=1000
IsVeinhole=true
```

[`IsVeinhole=yes`](/keys/isveinhole/) is what lets the mouse pick the monster out of the cell it stands in, and setting it also forces the type to be a legal target once its section has been read.

:::danger[The named type must resolve]
[`VeinholeTypeClass`](/keys/veinholetypeclass/) has no built-in value, and a monster reads its maximum strength straight off the type as it is created: a map carrying a veinhole overlay with the setting unresolved faults while it loads. A setting that names a section which does not exist fails more quietly — the type is created anyway and keeps the negative maximum strength it was constructed with, so the monster spawns below zero strength and every hit is refused before its armor is consulted at all.
:::

### Placement

Two paths create a monster. A map's overlay layer carries the veinhole overlay, which also stamps the eight surrounding cells with the dummy overlay the monster's artwork covers; or a map seed asks the random map generator for one. [`VeinholeMonsters`](/keys/veinholemonsters/) says how many to plant, and the generator makes at most 200 placement attempts in total, so a crowded map ends up with fewer. Each monster the generator plants is given a starting ring of veins around the border of its five-by-five block.

Outside scenario setup a monster is placed only where all of the following hold, tested in this order:

1. no other monster stands within two cells of the center on both axes at once;
2. the center cell stands at least one height level above the map floor;
3. every cell of the three-by-three block around the center is flat, of the `Clear` [land type](/reference/enums/land-type/), and at the center's own height.

A monster accepted there digs its pit: all nine cells drop one height level, the center stays flat, and the eight around it take ramps back up to the ground beyond.

While a scenario is still being set up that test is passed without any of those checks and the pit is not dug, so a monster a map places is accepted wherever the map puts it and keeps the ground the map drew for it.

### Idle, alert, and attacking

A monster steps through four states — idle, alert, attacking, and dying — and changes between them only at the one animation stage its current state allows, so a pending change waits for the animation to reach that stage. It moves from idle to alert while any cell of the five-by-five block around it holds an occupier — the building, vehicle, infantryman, landed aircraft or terrain object that a cell records as standing in it — and back to idle once that block is empty again. Terrain counts as readily as anything else, so a single tree inside the block holds a monster on alert for as long as the tree stands.

Damage is what makes it attack. Any hit that leaves it standing puts it into the attacking state and holds it there for 120 frames, 8 seconds; proximity alone never does. While the attack animation holds its open-mouth frame, one hard-coded `GasCloudM1` particle is created in the scenario's gas particle system at the monster's cell, 400 leptons above the ground and drifting upward and outward in a random direction. No further cloud is created until the animation moves off that frame.

### Destruction

An attack order aimed at the monster's cell resolves to the monster rather than to the ground, so a monster still standing takes directed fire like any other target.

:::caution[Splash damage reaches the monster only at its own cell]
An explosion looks the monster up at the blast's own cell. A blast centered one cell away still spills its damage into the veinhole cell for everything else standing there, but finds no monster to add and leaves it untouched.
:::

A destroyed monster enters its dying state, and at the end of that animation it replaces the veinhole and dummy overlays on its own cell and the four cells beside it with vein overlay, clears the four diagonal cells outright, then runs vein placement over its whole five-by-five block. The pit is left behind as ordinary vein-covered ground with no veinhole in it.

From then on the monster withers its field instead of growing it. Its queue is refilled with the mature cells it owns, scored by distance so that the cells farthest from the veinhole go first, and each step takes one ripeness step off 1 to 4 of them on a delay of [`VeinholeShrinkRate`](/keys/veinholeshrinkrate/) frames plus a random extra of up to half that figure. Withering is not gated by the scenario's growth switch. Once the queue is empty the monster is disposed of, and its disposal strips any vein overlay still standing in its five-by-five block.

## Vein fields

:::danger[The three vein overlays are fixed to their slots]
Veins, the veinhole, and the dummy ring are identified by their positions in `[OverlayTypes]` — 126, 167, and 178 — and not by anything in their sections. Overlay types are created in list order, so inserting, removing, or reordering entries ahead of them moves them off the slots the engine reads and breaks every part of this system.
:::

### Ripeness

A vein cell is either still growing — thin — or mature. Only a mature cell is harvested, damages what stands in it, carries the field further, and counts against a monster's coverage limit. A thin cell instead stores which of its four cardinal neighbors are mature, and a thin cell left with no mature cardinal neighbor loses its overlay altogether. A mature cell on a slope stores a piece cut for that slope; it is harvested and spread from like any other mature cell, but it never attacks.

:::note[Vein ripeness is not a Tiberium growth stage]
A vein cell has no growth stage and never climbs the numbered ripening a [Tiberium cell](/systems/tiberium/#cell-state) does. It is thin or it is mature, and every gameplay test asks only which; the two overlays keep their state in the same place and read it in unrelated ways.
:::

A vein overlay carrying [`Land=Weeds`](/keys/land/) makes its cell report that [land type](/reference/enums/land-type/) outright, ahead of whatever the ground tile underneath would say. That land type, not the overlay, is what the weed search and the loading test read.

### Growth

Each monster keeps its own growth timer. The first step falls due [`VeinholeGrowthRate`](/keys/veinholegrowthrate/) frames after the monster is created, and every later step reloads with that figure plus a random extra of up to half of it.

A step takes 1 to 5 cells off the monster's frontier — the queue of cells it has claimed to grow into but has not yet grown, held in the score order set out below — lowest score first. A cell not yet mature is made mature if it still accepts veins, and each cardinal neighbor that is not already mature vein is drawn into the field: as thin vein on flat ground, as a mature ramp piece on a slope. A cell that is mature then offers each of its four cardinal neighbors to the frontier — the field advances on the cardinals only, never diagonally — and a neighbor is queued when all of the following hold:

1. it lies inside the playable area;
2. its height differs from the source cell's by less than two levels;
3. it accepts veins;
4. no monster has claimed it already; and
5. the monster has handed out fewer than [`MaxVeinholeGrowth`](/keys/maxveinholegrowth/) frontier entries.

A queued cell is scored from the current frame divided by fifty plus a random figure of one to fifty. The base therefore rises by one every fifty frames, while two draws of the jitter can differ by as much as forty-nine — what the base takes 2,450 frames, nearly three minutes of play, to accumulate. Two cells queued anywhere inside that span can come out in either order, which is why the field advances unevenly rather than as a ring.

A step runs at all only while the monster has handed out no more than `MaxVeinholeGrowth` minus 40 frontier entries, covers no more than `MaxVeinholeGrowth` minus 100 mature cells, and the scenario's [`VeinGrowthEnabled`](/keys/veingrowthenabled/) switch is on. The [Vein growth](/mapping/actions/taction-vein-growth/) trigger action turns that switch on and off during play.

### What stops veins

A cell accepts veins when all of the following hold:

1. it is flat, or on one of the four standard ramps — the slopes that fall away toward one of the map's four directions, raising two of the cell's corners, as against the corner, steep and double ramp shapes;
2. its land type is not `Water`, `Rock`, `Ice`, or `Beach`;
3. it carries no overlay, or one declaring [`IsVeins=yes`](/keys/isveins/#scope-overlaytype); and
4. each of its four cardinal neighbors satisfies **all of**, in this order:

   - where the neighbor stands on a slope outside the four standard ramps while this cell is flat, it already carries an `IsVeins=yes` overlay;
   - its land type is outside that same list of four;
   - it carries no overlay other than an `IsVeins=yes` one.

The neighbor clause is the strict one: a field stops one cell short of a wall, a bridge, a crate, or a [Tiberium](/systems/tiberium/) cell even where the cell it would take is itself clear. The refusal runs both ways, since [Tiberium germinates](/systems/tiberium/#spread) only on a cell carrying no overlay at all.

### Loading a scenario

Every vein overlay in a map is stripped when the scenario finishes loading, and only the cells that were mature are placed again, which re-derives the connecting pieces around them from the ground as it now stands. A thin vein stored in a map file does not survive that pass.

The growth system is built afterward: each monster claims the connected field around its own cell, counts it, and queues the cells that can still take more. Any vein cell no monster claims by the end of that pass is removed.

## Standing in veins

A cell takes an attack when all of the following hold, tested in this order:

1. it carries vein overlay;
2. that vein is mature;
3. the cell is flat, so veins on a slope never take an attack;
4. no attack is already attached to it;
5. it holds a building, vehicle, infantryman or aircraft standing at height 5 or below that carries neither [`ImmuneToVeins=yes`](/keys/immunetoveins/) nor the `VEIN_PROOF` veteran ability.

That test is run when an object finishes a move into the cell by any means — driving, walking, stepping, hovering, teleporting, landing a jump jet, surfacing from a tunnel or a subterranean passage, or falling to the ground — when a flying object stops tumbling over it, when an object is placed onto the map over it, and when the cell itself ripens to mature vein under whatever is already standing there.

One trigger creates one attack for every vulnerable object standing in the cell at that moment, and each of them deals its full damage to everything there. A cell holding three vulnerable infantry therefore damages all of them three times over for as long as the attacks run.

The attack is the [`VeinAttack`](/keys/veinattack/) animation, and the animation does the damage itself: on every other frame it applies [`VeinDamage`](/keys/veindamage/) with [`VeinholeWarhead`](/keys/veinholewarhead/) to every object in its cell that is at height 5 or below and not immune. It removes itself once the cell's first occupier is gone or has left the ground, or once the cell stops carrying flat mature vein; removing it is what frees the cell to be triggered again.

:::danger[The attack animation must resolve and declare itself]
[`VeinAttack`](/keys/veinattack/) has no built-in value. With the setting unresolved, the first time a vulnerable object and mature flat vein meet in the same cell — whether the object arrives or the vein ripens under it — the cell builds an animation from a null type and faults. The animation type must also carry [`IsVeins=yes`](/keys/isveins/#scope-animtype) in `art.ini`: without it the animation plays as ordinary art, deals nothing, and never releases the cell's attack slot, so that cell can never be triggered again.
:::

:::caution[Veins with no warhead are harmless]
[`VeinholeWarhead`](/keys/veinholewarhead/) has no built-in value either. Damage carrying no warhead is modified to zero and dropped before any object's strength changes, so a field with the setting unresolved still plays its attack over every cell and takes nothing off anything.
:::

A warhead declaring [`Veinhole=yes`](/keys/veinhole/) resolves an attacker from the ground when the damage names none: the victim retaliates against the monster that owns the veins in the cell it is heading for, which is its own cell while it stands still. A computer-owned object retaliates in every case; a player-owned one does so while it has neither a target nor a movement order of its own.

## Weed harvesting

### Finding and loading

A UnitType with [`Weeder=yes`](/keys/weeder/#scope-unittype) runs the same mission as a [Tiberium harvester](/systems/tiberium/#harvesting) with the vein branch taken at each fork. It starts on that mission and returns to it on its own only while it carries no weapon: an armed weeder falls back to the guard behavior of an ordinary combat vehicle, and a player-owned weeder that goes idle away from vein ground is put on plain guard.

The patch search reads the same two distances as the Tiberium search, both in cells: [`TiberiumFarScan`](/keys/tiberiumfarscan/) when the weeder sets out with no patch in mind, and [`TiberiumNearScan`](/keys/tiberiumnearscan/) once it is working a field. The weeder takes its own cell when that already qualifies, and otherwise rings outward and takes the last qualifying cell of the first ring that yields any. A cell qualifies when all of the following hold, tested in this order:

1. it lies inside the playable area;
2. **Any of:** the match is not a campaign, the weeder does not belong to the local player, or the cell is not shrouded;
3. it sits in the same [movement zone](/glossary/#movement-zone) as the weeder's destination;
4. the weeder can enter it;
5. it reports the `Weeds` land type;
6. it holds mature vein.

```ini title="rules.ini"
[WEED] ; the stock weed eater
Weeder=yes
Dock=NAWAST
Storage=7
PipScale=Tiberium

[NAWAST] ; the stock weed refinery
Weeder=yes
PipScale=Tiberium
```

A loading cycle is nine stage ticks of three times [`HarvesterLoadRate`](/keys/harvesterloadrate/) frames each, so it runs at a third of the Tiberium cycle's pace. Each cycle takes one ripeness step off the cell the weeder stands on and adds one unit of weed — the counted quantity a compartment holds, not an object — to the weeder's first storage compartment, plus a second when the first did not fill it; [`Storage`](/keys/storage/) is the ceiling that decides when the weeder is full.

The cell is reduced through the monster that owns it, so a harvested cell drops back to thin vein and returns to that monster's frontier to be grown again. A vein cell no monster claims is demoted the same way, but nothing puts it back on a frontier at the time.

### Docking and unloading

A BuildingType with [`Weeder=yes`](/keys/weeder/#scope-buildingtype) is the only kind of building a weeder unloads at, and the weeder still reaches it through its own [`Dock`](/keys/dock/) list and its own house. Unlike a Tiberium refinery it does not need [`DockUnload=yes`](/keys/dockunload/): the weed branch of the docking request reads only the building's own flag and that nothing is attached to it yet. A weeder directed at another house's weed refinery may enter the bib and dock only when each house declares the other an ally; `Dock` does not make another building type compatible. Its docking cell is a fixed two-by-one offset from the building's own cell rather than a point at its center. A weed refinery carrying [`Bib=yes`](/keys/bib/) stops blocking that cell — on a three-cell-wide foundation the cell is the structure's own eastern edge and any vehicle drives over it, and only on a wider one is the pass narrowed to a mutually allied vein harvester. One that omits `Bib=` blocks the cell as it blocks any other cell it stands on.

Unloading is the refinery cycle with the animations left out. The weeder turns to face east and hands one unit to its house every [`HarvesterDumpRate`](/keys/harvesterdumprate/) minutes' worth of frames; no pre-production animation plays at the building, none is waited for once the weeder is empty, and a weeder never draws the harvesting artwork a Tiberium harvester draws. A weeder that is to change its artwork while it stands there names an [`UnloadingClass`](/keys/unloadingclass/); the rules-wide [`UnloadingHarvester`](/keys/unloadingharvester/) does not reach one. An emptied weeder takes the harvest mission again straight away.

### The weed pool

A house holds its weed apart from its Tiberium. Each unloaded unit is added one at a time up to [`WeedCapacity`](/keys/weedcapacity/), and everything past that figure is thrown away.

:::caution[Harvesting weeds earns no money]
Nothing turns weed into credits and nothing adds it to a house's score, in any game type and for a computer house as much as a player's — a contrast with the [Tiberium a harvester brings home](/systems/tiberium/#credits-and-storage). The pool has exactly one consumer: while a house holds exactly `WeedCapacity` units and owns a chemical missile superweapon that is not already charged, the superweapon is recharged and the whole pool is emptied in the same step. A house short of the full figure, or one harvesting without owning that superweapon, gets nothing at all for the trip.
:::

:::caution[Weed storage holds nothing until a capacity is set]
`WeedCapacity` is `0` when no rules file sets it, and that is the engine's own figure rather than an artifact of any shipped file. At `0` the store refuses the first unit offered to it, so a weeder loads, drives home, unloads, and the house's pool stays empty.
:::

A `Weeder=yes` building carrying [`PipScale=Tiberium`](/keys/pipscale/) shows the house's weed pool rather than its own contents, and its pip count is bounded by `WeedCapacity` instead of by its own `Storage`.

## Settings the engine parses but never reads

[`VeinholeMonsterStrength`](/keys/veinholemonsterstrength/) and [`VeinGrowthRate`](/keys/veingrowthrate/) are stored and never consulted. A monster's strength comes from the [`Strength`](/keys/strength/) of the section [`VeinholeTypeClass`](/keys/veinholetypeclass/) names, and growth timing comes from [`VeinholeGrowthRate`](/keys/veinholegrowthrate/).
