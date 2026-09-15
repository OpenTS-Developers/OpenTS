---
title: Random map generation
summary: "Builds a playable map out of a map seed's twenty settings through a fixed sequence of passes."
category: maps-scenarios
keys:
  - Accessibility
  - Biome
  - CliffRamps
  - Description
  - DirtRoadCurve
  - DirtRoadJunction
  - DirtRoadSlopes
  - Height
  - Ice1Set
  - Ice2Set
  - Ice3Set
  - IceShoreSet
  - IonAmbient
  - IonBlue
  - IonGreen
  - IonGround
  - IonLevel
  - IonRed
  - NumPlayers
  - PavedRoadEnds
  - PavedRoadSlopes
  - PavedRoads
  - RegionSize
  - RequiredForRMG
  - Ruggedness
  - Seed
  - Tiberium
  - TiberiumLayout
  - TiberiumWildlife
  - Time
  - UrbanPresence
  - UseBlueTiberium
  - UseIonStorms
  - UseTransitions
  - Vegetation
  - VeinholeMonsters
  - WaterAmount
  - WaterSet
  - Width
related:
  - type: format
    id: map-seed
  - type: format
    id: theater-control
  - type: system
    id: ion-storms
  - type: system
    id: tiberium
  - type: system
    id: veins
  - type: system
    id: crates
---

A map seed is not a map. It has no cells, no tiles and no waypoints. It holds twenty settings, and the generator turns those into terrain, water, cliffs, roads, buildings, tiberium and one starting position per player. [Map seed files](/formats/map-seed/) owns the file itself: how it is recognized, what its one section is called, and why the two size settings are indices rather than cell counts. This page owns what happens once it has been read.

A seed is one section holding the whole set:

```ini title="map seed file"
[RandomMap]
Description=Four-player temperate map
Width=2            ; size index; the tables turn it into a cell count
Height=2
NumPlayers=4       ; picks the row of minimum and maximum sizes, and the start-point count
Seed=90210         ; the state every random draw in every pass starts from
Biome=2            ; selects the theater and its light scale
Time=1             ; selects the hour's ambient level and the floodlight count
RegionSize=40      ; the dry-region size that splitting halves
Ruggedness=4       ; fixes the hill walk's step size and wander
Accessibility=50   ; the chance a region pair asks for extra ramps
WaterAmount=30     ; the budget the lake and river routines spend
Tiberium=10        ; how much tiberium each field receives
TiberiumLayout=50  ; how long the layout spread, and so how many fields, runs
Vegetation=50      ; scales the green and woods ground-cover chances
UrbanPresence=50   ; buys the settlements a biome admits
VeinholeMonsters=4 ; how many veinholes the pass asks for
TiberiumWildlife=1 ; scales each field's creature budget
UseIonStorms=yes   ; loads ION.INI and registers its triggers
UseBlueTiberium=no
UseTransitions=no  ; loads the hour's settings file and registers its triggers
```

Two things govern everything below. The passes run in one fixed order, and each reads a known handful of the twenty settings. A setting that appears to do nothing is usually a setting whose pass this particular seed never reaches. Every random choice in generation comes from one sequence started from [`Seed`](/keys/seed/), so two maps built from identical settings come out identical cell for cell. Changing any setting that alters how many draws an earlier pass makes changes everything after it too.

## The dialog path and the scenario path

A seed reaches the generator by two routes that share the passes and nothing else. The table sets both out. The last column matters most, because the map a player fights on is never the map the dialog built.

| Route | How the settings arrive | What is generated |
| --- | --- | --- |
| The map generator dialog | Taken off the dialog controls, rolled by its randomize button, or loaded from a saved seed and written back into the controls. All three of those actions hold every setting inside its legal range | A preview. The previous scenario is torn down first, down to its objects, houses, theater and tile artwork, and rebuilt around the new map. A thumbnail is drawn from the result |
| Starting a scenario whose filename ends `.SED` | Read straight out of the file's `[RandomMap]` section, over whatever the generator's settings currently hold. No range check runs | The map that is played. It is generated into the scenario already being loaded, with no teardown |

Accepting the dialog does not hand its map to the game. It writes the settings out to `RandMap.Sed` and registers that file in the scenario list. The lobby then starts `RandMap.Sed` as a scenario, and the second route builds the map over again from the same settings. The preview exists to be looked at, and is discarded.

The file extension carries weight beyond the generator, too. Recognizing `.SED` marks the scenario as a generated one for the rest of the load. That is what keeps a tile set marked [`RequiredForRMG=yes`](/keys/requiredforrmg/) resident when the loader trims artwork no cell is using yet.

:::danger[Nothing holds a seed file's settings inside their legal ranges]
The check that pins each setting to its limits belongs to the dialog. It runs at exactly three moments: when the dialog is read, when the dialog is filled in, and when its randomize button rolls a fresh set. Reading a seed file performs all twenty assignments and returns without it. A scenario started from a `.SED` therefore reaches the generator with whatever the file says. Several of those settings then index fixed tables that have no bounds check of their own. [`Biome`](/keys/biome/) selects the theater name that loads the tile set, [`NumPlayers`](/keys/numplayers/#scope-random-map-generation) selects the row of the size tables, and [`Time`](/keys/time/) selects the ambient light level. Loading the same file through the dialog is safe, because filling the controls in runs the check.
:::

## Sizing and the blank map

The first pass turns the seed into a blank scenario for the rest of them to work on.

The playable area is measured first. [`NumPlayers`](/keys/numplayers/#scope-random-map-generation) picks a row of minimum and maximum figures, and [`Width`](/keys/width/#scope-random-map-generation) and [`Height`](/keys/height/#scope-random-map-generation) each interpolate within that row. The table of figures is on the [map seed](/formats/map-seed/) page. The playfield is then written four cells wider and twelve taller, with the playable area inset two columns and five rows inside it. The border a generated map has is therefore fixed, and is not a function of any setting.

[`Biome`](/keys/biome/) chooses the theater, which is snow for tundra and taiga and temperate for the other three, and the light scale that theater uses. Every biome fills with the same `Clear` ground, so the fill is fixed too. [`Time`](/keys/time/) chooses the hour's ambient level. Those settings, the playfield, the playable area and a starting ground level of 4 are written into a scenario held in memory, which the scenario route then reads back as though it were a map file. What comes out is a map every cell of which is clear ground at one height. A per-cell working grid is allocated beside it to hold the region numbers, heights and ground-cover chances the later passes accumulate.

Two settings pull outside material in at this point, both of them before a single cell of terrain is placed:

- [`UseTransitions=yes`](/keys/usetransitions/) loads the settings file belonging to the hour (`MORNING.INI`, `DAY.INI`, `DUSK.INI` or `NIGHT.INI`) and registers its trigger types and tag types.
- [`UseIonStorms=yes`](/keys/useionstorms/) loads `ION.INI` and applies its `[General]` section over the loaded rules. It takes [`IonAmbient`](/keys/ionambient/#scope-random-map-generation), [`IonRed`](/keys/ionred/#scope-random-map-generation), [`IonGreen`](/keys/iongreen/#scope-random-map-generation), [`IonBlue`](/keys/ionblue/#scope-random-map-generation), [`IonGround`](/keys/ionground/#scope-random-map-generation) and [`IonLevel`](/keys/ionlevel/#scope-random-map-generation) from its `[Lighting]` section, and registers its trigger types and tag types. [Ion storms](/systems/ion-storms/#random-maps) covers what those triggers then do.

One draw is also made here rather than in the pass that uses it. Whether this map's rivers may end in a waterfall is rolled once, at one chance in four, and holds for the whole map.

:::caution[The snow theater's light cut does not reach the level an ion storm restores]
The ambient level written into the map is the hour's level cut by a quarter in the snow theater, and that is the level a generated map is lit at. The generator separately records the hour's level *without* that cut as the scenario's base ambient light. That is the value the end of an ion storm restores the map to. On a tundra or taiga map with `UseIonStorms=yes`, the first storm to pass therefore leaves the map permanently brighter than it began.
:::

## The passes in order

The table lists every pass in the order it runs, with the settings each one reads. The hills are raised late, after the tiberium, the towns and the roads are already down, so the ground those passes chose is not quite the ground that ends up beneath them. And [`Description`](/keys/description/#scope-random-map-generation) appears nowhere in it: it is the one assignment of the twenty that no pass reads.

| Pass | Settings it reads |
| --- | --- |
| Sizing and the blank map | `NumPlayers`, `Width`, `Height`, `Biome`, `Time`, `UseTransitions`, `UseIonStorms` |
| Water | `WaterAmount`, `Biome` |
| Ice smoothing | `Biome` |
| Regions, cliffs and ramps | `RegionSize`, `Accessibility`, `Biome` |
| Start points and the layout spread | `NumPlayers`, `TiberiumLayout` |
| Floodlights | `Time`, `UseTransitions`, `NumPlayers` |
| Veinholes | `VeinholeMonsters`, `Biome` |
| Tiberium | `Tiberium`, `NumPlayers`, `UseBlueTiberium`, `TiberiumWildlife` |
| Settlements | `UrbanPresence`, `Biome` |
| Hills | `Ruggedness` |
| Ground cover | `Vegetation`, `Biome`, `Width`, `Height` |

Most of those passes can be skipped outright, and nearly every condition that skips one is the biome. The table below gives each condition. A reader whose setting appears to do nothing can check first whether its pass ran at all.

| Pass or branch | Runs only when |
| --- | --- |
| The water pass as a whole | [`WaterAmount`](/keys/wateramount/) is not `0` |
| The river within it | `WaterAmount` is above `20`, and the biome is not desert |
| The arctic lake and river, which lay ice | The biome is tundra; taiga uses the ordinary ones |
| Ice smoothing | The biome is tundra |
| Region splitting, which is where a generated map's cliffs come from | The biome is **not** tundra |
| Veinholes | The theater is not arctic, so never on tundra or taiga |
| Creatures in a tiberium field | The Firestorm addon is present, and the field is not one of the start-point fields |
| Urban areas | The biome is temperate, desert or mutated |
| Rural settlements | The biome is tundra or taiga |
| Mold and crystal growths | The biome is mutated |
| The hill height walk | [`Ruggedness`](/keys/ruggedness/) is at least `2` |

## Water

The water pass spends a budget rather than filling a proportion. The budget is [`WaterAmount`](/keys/wateramount/) multiplied by the playable area and by a factor chosen per biome, with a hundred cells added on top. The lake and river routines spend against it and stop when it runs out.

A river is attempted first, up to ten times, and only on a wet enough map that is not desert. A lake follows, also up to ten times. Tundra uses arctic variants of both, which lay ice rather than open water; the ice smoothing pass afterwards has to blend them into the ground around them. Taiga is also a snow-theater map, but it gets the ordinary lake and river and no ice at all. On the mutated biome up to two stretches of a lake are then soured into swamp. Both routines read the theater's [`WaterSet`](/keys/waterset/) to decide what already counts as water. A river that reaches a drop in height may finish in a waterfall, where the once-per-map roll allowed one.

The pass leaves the map's water recorded as regions in the working grid rather than as loose cells. That is what lets the next pass treat a lake as a single object and span it with bridges.

## Regions, cliffs and ramps

A **region** here is a group of connected cells that the generator holds at one ground height. The pass builds them in three stages.

The water just laid becomes regions of its own first, each grown four rings outward and allowed to absorb the ground it reaches. Everything left over is then carved into land regions. Finally, any dry region larger than the limit [`RegionSize`](/keys/regionsize/) sets is split in two, with one half moved to a new height, and the splitting repeats until nothing oversized is left. The cliff faces of a generated map are a by-product of that last stage; nothing places a cliff directly. The stage is skipped altogether on tundra, which is why the setting changes nothing there.

Once the heights have settled, every region works out which regions it borders and connects itself to them. A water region spans itself with bridges to the dry land on either side, and only where both sides stand at the water's own height. A dry region instead carves ramps down to each lower neighbor. One ramp is always asked for, and [`Accessibility`](/keys/accessibility/) is the chance, rolled once per pair, that the pair asks for one or two more. The artwork comes from the theater, and the roles that describe rock face ([`CliffRamps`](/keys/clifframps/) among them) are what tell the generator which tiles it must not build over while it works.

## Start points and the layout spread

The start points are chosen only now, out of the finished terrain, and they are chosen together with the tiberium field sites.

Fifteen candidate cells per player are gathered from the large regions that are mutually reachable. Each candidate needs a ten-by-ten block that a paved road could be laid in, and a position at least four cells inside the playable area. The most widely separated of those candidates are then picked out as a single spread, with cells lying in different regions treated as further apart than they are, so that the picks favor separate pieces of ground. The first [`NumPlayers`](/keys/numplayers/#scope-random-map-generation) cells of that spread become the start waypoints. What remains of it is the tiberium layout, and [`TiberiumLayout`](/keys/tiberiumlayout/) is what decided how long the spread was to begin with.

Each start point then has to flood exactly four hundred clear cells around itself. Every cell that flood reaches is marked **protected**, a per-cell flag the later placement passes avoid. That is what keeps towns, veinholes and ground cover out of a player's opening room. If any player's flood runs dry short of four hundred, the whole selection is thrown away and made again from the next state of the random sequence, with the terrain kept as it stands. Nothing caps that retry.

## Floodlights, veinholes and tiberium

Three placement passes follow, in this order.

The floodlight pass rings each start point with civilian lights, in a number [`Time`](/keys/time/) fixes and [`UseTransitions=yes`](/keys/usetransitions/) overrides. A ring goes down only where every light in it can legally stand, and twenty-one ring angles are tried before a start point is left dark.

The veinhole pass asks for [`VeinholeMonsters`](/keys/veinholemonsters/) monsters, with at most two hundred attempts made in total, so a crowded map simply ends up with fewer of them. Each attempt draws a cell and accepts it only when all of these hold, in this order:

1. The five-by-five block centered on the cell, and the ring of cells around that block, lie within the playable area.
2. **All of:**
   - no cell of the block is protected ground;
   - every cell of the block is clear tile, stands at the block's own height, and has no overlay.
3. The [veinhole placement test](/systems/veins/#placement) accepts the cell.

An accepted cell takes the veinhole overlay, its eight neighbors take the dummy overlay, and the border of the block is given a starting ring of veins.

The tiberium pass grows every cell of the layout spread into a field, and then gives each player a compensating field at their own start point. That field's size is the difference between the player's mean distance to the layout sites and the smallest such mean among all the players, multiplied by fifteen, plus a flat five hundred. The player sitting farthest from the map's tiberium therefore gets the largest field on their doorstep, and the closest player gets the bare five hundred. [`Tiberium`](/keys/tiberium/#scope-random-map-generation) sets how much tiberium each field receives, and [`TiberiumLayout`](/keys/tiberiumlayout/) sets how many fields the spread leaves.

A field grows outward from its site over open ground, thickening tiberium already there rather than overwriting it, and starts over from its origin up to ten times if the growth stalls early. Which overlay set it grows is settled per field. With [`UseBlueTiberium`](/keys/usebluetiberium/) clear, every field grows the first set; with it set, each layout field is rolled for separately. The start-point fields are handled differently. One roll covers all of them, so every player's opening field grows the same set as every other player's, and that same roll also decides whether a tiberium tree is planted at the heart of each.

:::danger[A field's creature budget is either nothing or more than it can spend]
[`TiberiumWildlife`](/keys/tiberiumwildlife/) scales a budget drawn once per field from five outcomes. Two of the five were meant to come out negative and be discarded, but the subtraction runs in unsigned arithmetic and wraps them to about four thousand million instead. At the `30` the map generator dialog's check box writes, the three surviving outcomes truncate to no creatures at all. The two wrapped ones scale to something above a thousand million, a budget no field can exhaust, so the field goes on turning a creature loose at intervals for as long as it keeps growing. A field therefore gets either nothing or a stream, and the one or two creatures the draw was shaped to give never occur. That is not the same as nothing ever being placed: a map built at the check box's figure does have creatures, on roughly two of its spread fields in five.
:::

## Settlements

[`UrbanPresence`](/keys/urbanpresence/) buys different things on different biomes. Tundra and taiga get rural settlements: a dirt road junction on open unprotected ground, a handful of civilian buildings along the roads grown from it, and a few civilian vehicles. Temperate, desert and mutated get urban areas: a paved district flood-filled outward from a cell, with its own road network, buildings, traffic and a pavement blend around the edge. Both branches give up after ten attempts, however many were asked for. The key page covers what that ceiling does to the range.

Both draw their road pieces from the theater. The rural branch works from a library counted off [`DirtRoadCurve`](/keys/dirtroadcurve/) and opens each network on a shape from [`DirtRoadJunction`](/keys/dirtroadjunction/). The urban branch lays numbered pieces counted off [`PavedRoads`](/keys/pavedroads/) and caps each run with a piece from [`PavedRoadEnds`](/keys/pavedroadends/). Neither branch ever selects a ramp piece. That is why [`DirtRoadSlopes`](/keys/dirtroadslopes/) and [`PavedRoadSlopes`](/keys/pavedroadslopes/) are inert, and a generated road stops at a slope rather than climbing it.

:::caution[The urban branch can never fall back to a rural settlement]
The urban pass has a branch that would place a rural settlement instead, taken against a chance of one minus the setting. The setting is a whole number from `0` to `100` rather than a fraction. That chance is zero at `UrbanPresence=1` and negative above it, so the branch is never taken. At `UrbanPresence=0` the pass asks for nothing and stops before it could be. A temperate, desert or mutated map therefore only ever receives urban areas.
:::

## Hills and ground cover

The hills come after all of the above. Cells beside a shoreline or a cliff face are pinned down first, so the walk that follows cannot raise a hill into the water or bury an existing cliff. That walk then sweeps the map, giving each cell a height drawn from the neighbors already settled. [`Ruggedness`](/keys/ruggedness/) fixes both the size of each step and how far the walk may wander around it. The terrain is finally stepped up or down one level at a time to meet the result, with the smoothing system cutting slopes and ramps in as it goes. Below `Ruggedness=2` the walk is skipped and no cell is given an offset at all.

Ground cover is last. Each cell is first given four chances: green ground, rough ground, sand and woods. [`Vegetation`](/keys/vegetation/) scales the green and the woods alone, and those two are multiplied tenfold again along a shoreline. Each cell is then rolled against the four in that order, and the first to come up decides what the cell gets. A cell is considered only while it is clear tile, has no ramp and no overlay, has nothing standing on it and is not protected ground. A mutated map has its mold and crystal growths laid down before this, in numbers derived from the two size indices, and desert and mutated maps also get a scatter of loose rough ground first. The pass finishes by fixing up the transition tiles and littering the map with rock overlays.

## What the theater must supply

The generator has no terrain artwork of its own; it works entirely through the roles a theater control file declares. The table groups those roles by the pass that needs them, so a modder adding or renumbering tile sets can see which part of generation a missing role would reach. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

| Pass | Roles it reads |
| --- | --- |
| Water and ice | [`WaterSet`](/keys/waterset/), the shore, waterfall and swamp sets, [`Ice1Set`](/keys/ice1set/), [`Ice2Set`](/keys/ice2set/), [`Ice3Set`](/keys/ice3set/), [`IceShoreSet`](/keys/iceshoreset/) |
| Regions, cliffs and ramps | The cliff, slope and ramp sets, [`CliffRamps`](/keys/clifframps/) among them |
| Settlements | [`PavedRoads`](/keys/pavedroads/), [`PavedRoadEnds`](/keys/pavedroadends/), [`DirtRoadCurve`](/keys/dirtroadcurve/), [`DirtRoadJunction`](/keys/dirtroadjunction/), and the pavement and median sets |
| Ground cover | The green, rough, sand, rock, mold and crystal sets |

Tile artwork is trimmed as a map is read, by counting how many cells use each type and discarding the artwork of every type the count leaves at zero. A generated map places most of its tiles after that count has been taken, which is what [`RequiredForRMG=yes`](/keys/requiredforrmg/) exists for. It exempts a set from the trim, but only on a map the loader knows is generated, and recognizing the `.SED` extension is what tells it that.

## Handing the map over

When the last pass finishes, the vein and tiberium growth and spread systems are started, and the working grid and the region records are thrown away. The map's overpass and radar image are computed, and the finished map is handed back to the scenario load already in progress.

Only then do the multiplayer fixups run. They are deliberately held back while a generated scenario is being read, because until generation finishes there is nothing for them to work on. Each house's starting force is created at the start waypoints the generator wrote, [crates](/systems/crates/) are scattered if the session asked for them, and computer houses take their credit multipliers. A generated map is an ordinary scenario from that point on.

Two lobby-side reads happen before any of this, and both look at the seed file rather than at a map. The multiplayer and skirmish lobbies check that the chosen scenario has somewhere to put everybody by counting its `[Waypoints]` entries. They fall back to reading [`NumPlayers`](/keys/numplayers/#scope-random-map-generation-2) when it has none, which is always true of a seed file, whose start points do not exist until the map is generated. A saved seed is identified, both in the lobby and in the dialog's preview cache, by a digest taken over the settings that stops short of [`UseTransitions`](/keys/usetransitions/). Two seeds differing in nothing but that flag are treated as the same map.
