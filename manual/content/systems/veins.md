---
title: Veins and veinhole monsters
summary: "Veinhole monsters grow vein fields that damage objects standing in them; weeders harvest the veins to charge the chemical missile."
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

A veinhole monster sits in one cell and grows a field of veins around itself. It belongs to no house and is neither a building nor a vehicle. Every monster uses the TerrainType named by [`VeinholeTypeClass`](/keys/veinholetypeclass/) in `[General]`, so that type's [`Strength`](/keys/strength/), [`Armor`](/keys/armor/) and [`Immune`](/keys/immune/) settings apply to all of them. The monster is drawn from the current theater's `VEINHOLE` shape file, such as `VEINHOLE.TEM`, not from the type's `Image`.

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

Set [`IsVeinhole=yes`](/keys/isveinhole/) on that TerrainType so that players can click the monster. The flag also makes the type a legal target, even if its section sets `LegalTarget=no`.

:::danger[Name an existing TerrainType]
If `VeinholeTypeClass` is not set, the game crashes when it creates a monster: while it loads a map containing a veinhole, or while it generates a random map that plants one. If the setting names a section that does not exist, every monster starts below zero strength and cannot be damaged.
:::

### Placement

A map places a monster with the `VEINHOLE` overlay on its center cell. When the map loads, the engine fills the eight surrounding cells with the `VEINHOLEDUMMY` overlay itself. It neither checks nor reshapes the ground under a monster placed this way, so draw the pit in the map if the monster should sit in one.

The random map generator plants the number of monsters set by [`VeinholeMonsters`](/keys/veinholemonsters/). It makes at most 200 placement attempts in total, so a crowded map gets fewer. Each attempt picks a site as that page describes, and a monster is then placed only where all of the following hold, tested in this order:

1. no other monster stands inside the five-by-five block centered on the site;
2. the center cell is at height 1 or higher;
3. every cell of the three-by-three block around the center is flat, of the `Clear` [land type](/reference/enums/land-type/), and at the center's height.

The generator then digs the monster's pit: all nine cells drop one height level, the center stays flat, and the eight cells around it slope back up to the surrounding ground. It also puts mature veins on the outer ring of the monster's five-by-five block, wherever that ground accepts them.

### Idle, alert, and attacking

A monster is alert while any cell of the five-by-five block centered on it holds a building, vehicle, infantry, landed aircraft or terrain object, and idle when the block is empty. A single tree in the block keeps the monster alert for as long as the tree stands. Being alert changes only the monster's animation.

A monster attacks only after it is damaged. A hit that damages it without destroying it makes it attack for 120 frames (8 seconds); a hit that deals no damage does not. While attacking, the monster releases one particle of the `GasCloudM1` type, whose name is fixed, each time its mouth opens fully. The particle appears 400 leptons above the monster's cell, drifts away in a random direction, and joins the scenario's shared [gas cloud system](/systems/particle-systems/#systems-that-no-attachment-holds). The particle type decides what the cloud does; in the shipped rules it becomes a `GasCloud1` cloud, which damages objects beneath it.

A change of state waits until the monster's current animation reaches a point where it can switch, so the monster can react a moment late.

### Destruction

Players can attack the monster by clicking it. Vehicles, infantry and aircraft ordered to attack its cell also attack the monster itself. A building given that order fires at the ground in the cell instead.

:::caution[Splash damage reaches the monster only in its own cell]
An explosion damages the monster only when it is centered on the monster's cell, and only if the `VEINHOLE` overlay sets [`IsVeinholeMonster=yes`](/keys/isveinholemonster/), as the shipped one does. An explosion in a neighboring cell still damages other objects in the monster's cell, but not the monster.
:::

A destroyed monster plays its dying animation and then turns its pit into part of the field. Its own cell and the four cells beside it take vein overlay, and the four diagonal cells lose their overlay. Veins are then placed across the five-by-five block wherever the ground accepts them, which leaves the pit's corner slopes bare.

The field then withers. Every [`VeinholeShrinkRate`](/keys/veinholeshrinkrate/) frames, plus a random extra of up to half as many, 1 to 4 of the monster's mature cells turn thin or lose their vein, starting with those farthest from the veinhole. Each mature cell withers once, and thin veins disappear once no mature cell remains next to them. Withering continues even while [`VeinGrowthEnabled`](/keys/veingrowthenabled/) is off. When every mature cell has withered, the monster is removed, together with any vein left in its five-by-five block, even vein another monster owns. Veins outside that block stay on the map.

## Vein fields

:::danger[Keep the vein overlays at their positions]
The engine finds the `VEINS`, `VEINHOLE` and `VEINHOLEDUMMY` overlays by their positions in `[OverlayTypes]`, not by anything in their sections. The shipped rules list them at positions 126, 167 and 178, counting entries from 0 in the order they appear; the numbers left of `=` do not matter. Add new overlay types after them, and do not insert, remove or reorder entries before them. If one of the three moves, the engine treats whichever overlay now holds its position as that overlay, and the vein system stops working.
:::

### Ripeness

A vein cell is either thin or mature. Only mature veins spread the field, attack objects, and can be harvested. Thin veins are the connecting pieces drawn beside mature cells. Veins have no numbered growth stages like a [Tiberium cell](/systems/tiberium/#cell-state).

Harvesting or withering a mature cell makes it thin, or removes its vein if none of its four cardinal neighbors is mature. A thin neighbor left with no mature cardinal neighbor loses its vein as well.

A mature vein on a slope spreads the field and can be harvested, but it never attacks, and harvesting removes it instead of making it thin.

The shipped `VEINS` overlay sets [`Land=Weeds`](/keys/land/), which gives its cell the `Weeds` [land type](/reference/enums/land-type/) whatever the ground tile beneath it. Weeders look for that land type when they search for veins and when they load, so they cannot harvest a vein overlay without it.

### Growth

Each monster grows its own field in steps. The first step comes [`VeinholeGrowthRate`](/keys/veinholegrowthrate/) frames after the monster is created. Each later step comes that many frames after the previous one, plus a random extra of up to half as many.

A step takes the next 1 to 5 cells from the monster's growth queue. Each cell taken becomes mature if it still accepts veins.

A cell that matures on flat ground also puts vein on each cardinal neighbor that does not already hold mature vein or a `VEINHOLE` or `VEINHOLEDUMMY` overlay. A flat neighbor gets thin vein, and a sloped neighbor gets mature vein at once.

Each cell taken that is mature after the step, whether it has just matured or already was, then adds its cardinal neighbors to the queue. A neighbor is added when all of the following hold:

1. it lies inside the playable area;
2. its height differs from the mature cell's by less than two levels;
3. it [accepts veins](#what-stops-veins);
4. no monster owns it yet.

Veins spread only to cardinal neighbors, never diagonally. Cells leave the queue roughly in the order they joined it, but a random offset lets two cells queued up to 2,450 frames apart, nearly three minutes, grow in either order. A field therefore spreads unevenly instead of as an expanding ring.

The monster that queues a cell owns it. That monster regrows the cell when harvesting leaves thin vein in it, and withers it after the monster is destroyed.

A growth step runs only while all of the following hold:

1. the monster has queued no more than [`MaxVeinholeGrowth`](/keys/maxveinholegrowth/) minus 40 cells in total;
2. it covers no more than `MaxVeinholeGrowth` minus 100 mature cells, its coverage limit; and
3. the scenario's `VeinGrowthEnabled` switch is on. The [Vein growth](/mapping/actions/taction-vein-growth/) trigger action turns it on and off during play.

The queued total is a budget for the whole scenario. It never goes down, a saved game keeps it, and a harvested cell queued to grow back also adds to it. Once the total passes `MaxVeinholeGrowth` minus 40, the monster stops growing for the rest of the scenario, and cells harvested after that stay thin. No cell is ever queued past `MaxVeinholeGrowth` itself.

### What stops veins

A cell accepts veins when all of the following hold:

1. it is flat, or on one of the four simple slopes, which raise two adjacent corners of the cell. Every other slope shape refuses veins;
2. its land type is not `Water`, `Rock`, `Ice` or `Beach`;
3. it has no overlay, or an overlay with [`IsVeins=yes`](/keys/isveins/#scope-overlaytype). The shipped `VEINS`, `VEINHOLE` and `VEINHOLEDUMMY` overlays all set it; and
4. each of its four cardinal neighbors meets **all of** the following, in this order:

   - if this cell is flat, the neighbor is flat, on a simple slope, or has an `IsVeins=yes` overlay;
   - its land type is not `Water`, `Rock`, `Ice` or `Beach`;
   - it has no overlay, or an `IsVeins=yes` overlay.

The neighbor test is why a field stops one cell short of a wall, a bridge, a crate or [Tiberium](/systems/tiberium/), even where the cell it would grow into is clear. Tiberium is kept out in return, because it [spreads](/systems/tiberium/#spread) only onto cells with no overlay.

### Loading a scenario

Store vein fields in a map as mature cells. While a scenario loads, right after its overlays are read, the engine removes every vein overlay and places veins again on each stored mature cell that still accepts them. The thin pieces around those cells are redrawn to fit the ground, and thin veins stored in the map are discarded. A cell's ripeness is stored in the map's [overlay packs](/formats/scenario-terrain/#overlay-packs).

Each monster then takes ownership of the vein field connected to its five-by-five block, counting diagonal connections, and queues the thin cells in that field to grow first. Vein cells that no monster owns are removed, so a map without monsters loses all its veins. A monster with no vein in its five-by-five block never grows, so a map that places a monster needs mature veins in that block.

The mature cells a monster takes over count toward its [coverage limit](#growth), and the cells it queues count toward its queued total. A monster that takes over more than `MaxVeinholeGrowth` minus 100 mature cells, 1,900 with the shipped rules, therefore never grows.

## Standing in veins

Flat, mature veins attack buildings, vehicles, infantry and aircraft standing in them. An object is not attacked while it is more than 5 leptons above the ground, when its type sets [`ImmuneToVeins=yes`](/keys/immunetoveins/), or when it has the [`VEIN_PROOF`](/systems/veterancy/#abilities) veteran ability. Thin veins and veins on a slope never attack.

A cell checks for vulnerable objects, and starts an attack if it finds any, whenever an object finishes moving into it, however it travels, or is placed on the map there. It checks again when its vein matures under objects already standing in it. A cell that already has an attack running starts no new one.

The attack is the [`VeinAttack`](/keys/veinattack/) animation. On every other frame, it deals [`VeinDamage`](/keys/veindamage/) with [`VeinholeWarhead`](/keys/veinholewarhead/) to every vulnerable object in its cell. It ends when the cell is empty, when the cell no longer holds flat mature vein, or when the object that entered the cell last is off the ground. The cell can then start another attack.

A cell starts one animation for each vulnerable object standing in it, and each animation damages every vulnerable object there. Three vulnerable infantry in one cell therefore each take three hits every other frame.

:::danger[Configure the vein attack animation]
Set `VeinAttack` to a valid animation type. If it is not set, the game crashes the first time veins attack, whether an object enters mature vein or a vein matures under one.

Set [`IsVeins=yes`](/keys/isveins/#scope-animtype) in that animation's `art.ini` section. Otherwise the animation deals no damage, and its cell never starts another attack.
:::

:::caution[Set a vein warhead]
If `VeinholeWarhead` is not set, vein attacks still play but deal no damage.
:::

If the vein warhead sets [`Veinhole=yes`](/keys/veinhole/), a damaged object fights back against the monster that owns the veins where it stands, or, if it is moving, where it steps next. A computer-owned object always does. A player-owned object does so only while it has no target and no movement order, and only when the monster is within its weapon range or sight.

## Weed harvesting

### Finding and loading

A UnitType with [`Weeder=yes`](/keys/weeder/#scope-unittype) harvests veins through the same harvest mission as a [Tiberium harvester](/systems/tiberium/#harvesting). It starts harvesting when it is placed on the map, including when it leaves a factory or a repair bay. When it later goes idle, it resumes harvesting if its house is computer-controlled or it is standing on veins. A player-owned weeder that goes idle anywhere else takes a guard mission instead, so a player can park it. A weeder whose house owns none of the buildings in its [`Dock`](/keys/dock/) list switches to guard. Do not also set [`Harvester=yes`](/keys/harvester/#scope-unittype): such a vehicle waits for Tiberium ground instead, as [Tiberium harvesting](/systems/tiberium/#harvesting) explains.

A weeder first checks its own cell, then searches outward one ring at a time and takes a qualifying cell from the nearest ring that has one. The search covers every ring closer than [`TiberiumFarScan`](/keys/tiberiumfarscan/) cells when the weeder sets out, and closer than [`TiberiumNearScan`](/keys/tiberiumnearscan/) cells while it works a field. A cell qualifies when all of the following hold, tested in this order:

1. it lies inside the playable area;
2. in a campaign, if the local player owns the weeder, the cell is not shrouded;
3. it is in the same [movement zone](/glossary/#movement-zone) as the cell the weeder is heading to, or its own cell when it is not moving;
4. the weeder can enter it now, so a cell with a vehicle parked in it does not qualify;
5. it has the `Weeds` land type;
6. it holds mature vein.

```ini title="rules.ini"
[WEED] ; the stock weed eater
Weeder=yes
Dock=NAWAST
Storage=7
PipScale=Tiberium
ImmuneToVeins=yes  ; otherwise the veins it harvests damage it

[NAWAST] ; the stock weed refinery
Weeder=yes
Bib=yes
PipScale=Tiberium
```

A weeder harvests a mature cell in one go. Each cell gives two units of weed, or one if the first unit fills the weeder; [`Storage`](/keys/storage/) sets how many units it can carry. The cell then turns thin or loses its vein, as described under [Ripeness](#ripeness). After each cell, the weeder waits 27 times [`HarvesterLoadRate`](/keys/harvesterloadrate/) frames before it moves on, and it harvests the next cell no sooner than 9 times `HarvesterLoadRate` frames after it leaves the previous one.

A harvested cell that is left with thin vein and belongs to a live monster rejoins that monster's growth queue and can mature again, within the limits under [Growth](#growth). A harvested cell that loses its vein, or that no monster owns, is not queued to grow back. A monster's field can still spread into it again from a neighboring cell that matures.

### Docking and unloading

A weeder unloads only at a BuildingType with [`Weeder=yes`](/keys/weeder/#scope-buildingtype), which needs no [`DockUnload=yes`](/keys/dockunload/). A weeder returning on its own looks only among its own house's buildings whose types are in its `Dock` list, so that list must name at least one weed refinery. A player can also send a weeder into another house's weed refinery, but only when each house is allied with the other.

The weeder docks on a fixed cell of the building's [`Foundation=WxH`](/keys/foundation/#scope-buildingtype): the third cell along `W` and the second along `H`, counting from the foundation's top corner. Give the building [`Bib=yes`](/keys/bib/) so that vehicles can drive onto that cell:

- With a foundation three cells wide, such as the `3x3` of the shipped `NAWAST`, the dock cell is on the building's last column, and any vehicle can drive onto it.
- With a wider foundation, only a weeder whose house and the building's house are allied with each other can enter it.
- With a foundation narrower than three cells, or only one row deep, the dock cell lies outside the building.

Without `Bib=yes`, a dock cell inside the foundation is blocked like the rest of the building, so no weeder can drive onto it.

A docked weeder faces east and hands its house one unit every [`HarvesterDumpRate`](/keys/harvesterdumprate/) minutes of game time. Unlike a Tiberium refinery, the building plays no pre-production animation, and the weeder does not wait for a production animation to finish. To change the weeder's artwork while it unloads, set [`UnloadingClass`](/keys/unloadingclass/) on its type; the rules-wide [`UnloadingHarvester`](/keys/unloadingharvester/) applies only to Tiberium harvesters. A weeder also never shows the harvesting artwork a Tiberium harvester shows. Once it is empty, the weeder returns to harvesting.

### The weed pool

Each house stores weed separately from Tiberium, up to [`WeedCapacity`](/keys/weedcapacity/) units. Weed unloaded while the pool is full is lost.

:::caution[Weed earns no credits]
Weed never becomes credits or score, for any house in any game type. Its only use is charging a chemical missile superweapon.
:::

When a house's pool holds exactly `WeedCapacity` units and the house owns a `Type=ChemMissile` superweapon that is not ready, the pool is emptied and the weapon's countdown restarts at its full [`RechargeTime`](/keys/rechargetime/). A pool below `WeedCapacity` does nothing.

A full pool restarts a countdown that is already running, so the progress made so far is lost. While the weapon is ready, a full pool waits and further weed is lost.

The shipped chemical missile sets [`ManualControl=yes`](/keys/manualcontrol/), so it does not charge at all until its house's pool fills. [Manual control](/systems/superweapons/#manual-control) explains the setting.

:::caution[Set a weed capacity]
Set `WeedCapacity` above `0`. While it is `0` or not set, the pool refuses every unit, so weeders unload and nothing is stored.
:::

A `Weeder=yes` building with [`PipScale=Tiberium`](/keys/pipscale/) shows how full its house's weed pool is, not its own contents. It shows at most `WeedCapacity` pips, and fewer if its [`MaxPips`](/keys/maxpips/) or foundation leaves room for fewer.

## Settings the engine parses but never reads

[`VeinholeMonsterStrength`](/keys/veinholemonsterstrength/) and [`VeinGrowthRate`](/keys/veingrowthrate/) have no effect. Set a monster's strength with [`Strength`](/keys/strength/) on the type [`VeinholeTypeClass`](/keys/veinholetypeclass/) names, and the time between growth steps with [`VeinholeGrowthRate`](/keys/veinholegrowthrate/).
