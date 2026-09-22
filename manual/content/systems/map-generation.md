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

The random map generator builds a map from a map seed. A seed holds twenty settings and no map data: no cells, no tiles and no waypoints. From those settings the generator makes the terrain, water, cliffs, roads, buildings, tiberium and one start point per player. [Map seed files](/formats/map-seed/) covers the file itself, including how it is recognized and why the two size settings are indices, not cell counts. This page covers what the generator does with the settings.

A seed is one `[RandomMap]` section holding all twenty:

```ini title="map seed file"
[RandomMap]
Description=Four-player temperate map
Width=2            ; size index from 0 to 3, not a cell count
Height=2
NumPlayers=4       ; start points, and the row of map sizes Width and Height pick from
Seed=9021          ; starts the generator's random sequence
Biome=2            ; theater and ground cover; 2 is temperate
Time=1             ; ambient light and floodlights; 1 is afternoon
RegionSize=40      ; how large a piece of ground may stay at one height
Ruggedness=4       ; how far hills rise and fall
Accessibility=50   ; chance that two regions get extra ramps between them
WaterAmount=30     ; how much water the river and lake may lay
Tiberium=10        ; how much tiberium the fields across the map receive
TiberiumLayout=50  ; how many tiberium fields are spread across the map
Vegetation=50      ; chance of green ground and woods
UrbanPresence=2    ; how many settlements are attempted
VeinholeMonsters=4 ; how many veinhole monsters are attempted
TiberiumWildlife=30 ; creature budget of each tiberium field
UseIonStorms=yes   ; loads ION.INI
UseBlueTiberium=no
UseTransitions=no  ; loads the hour's settings file
```

The passes run in a fixed order, and each reads only a few of the settings. If a setting seems to do nothing, check whether this seed skips the pass that reads it. [The passes in order](#the-passes-in-order) lists the conditions.

The generator makes every random choice from one sequence started from [`Seed`](/keys/seed/). The same settings with the same game data build the same map, cell for cell. A setting that changes how many random choices an early pass makes also changes every pass after it, so a small change can reshape the whole map.

## The dialog path and the scenario path

A seed reaches the generator by one of two routes. Both run the same passes, but only the scenario route builds a map that is played.

| Route | Where the settings come from | What is built |
| --- | --- | --- |
| The map generator dialog | The dialog's controls. Loading a saved seed or pressing the randomize button fills the controls first | A preview and its thumbnail. The dialog clears the previous map before each build |
| A scenario whose filename ends `.SED`, or whose `[Basic]` section sets [`RandomMap=yes`](/keys/randommap/) | The file's `[RandomMap]` section. A setting the section leaves out takes its default. A `RandomMap=yes` file uses the match's seed instead of the section's `Seed` | The map that is played, built while the scenario loads. The `RandomMap=yes` page explains which of the file's other sections still apply |

The preview is never played. Accepting the dialog saves its settings to `RandMap.Sed` and adds that file to the lobby's scenario list. When the match starts, the scenario route builds the map again from those settings.

Accepting the dialog builds a preview only when none has been built since the dialog opened. Otherwise it reuses the last preview. If a setting changed after the last preview, the lobby thumbnail still shows the older map, while the match uses the new settings.

Both routes hold every setting to its legal range before a map is built. The dialog does this whenever it reads its controls, fills them in or rolls a random set. The scenario route does it after reading the file. Each setting's key page gives its range.

## Sizing and the blank map

The first pass builds the blank map that the other passes work on.

[`NumPlayers`](/keys/numplayers/#scope-random-map-generation) selects a row of minimum and maximum sizes, and [`Width`](/keys/width/#scope-random-map-generation) and [`Height`](/keys/height/#scope-random-map-generation) each pick a point within that row. [Map seed files](/formats/map-seed/) has the table. The result is the playable area. The playfield is four cells wider and twelve cells taller, with the playable area inset two columns and five rows. No setting changes that border.

[`Biome`](/keys/biome/) selects the theater: snow for tundra and taiga, temperate for the other three. [`Time`](/keys/time/) selects the ambient light level. Every cell starts as `Clear` ground at height 4, whatever the biome.

Two settings load extra files before any terrain is placed:

- [`UseTransitions=yes`](/keys/usetransitions/) loads the hour's settings file: `MORNING.INI`, `DAY.INI`, `DUSK.INI` or `NIGHT.INI`. The map takes its trigger types, tag types and local variables. Its local variables replace any that the scenario file named.
- [`UseIonStorms=yes`](/keys/useionstorms/) loads `ION.INI` and applies its `[General]` section over the loaded rules. The map takes [`IonAmbient`](/keys/ionambient/#scope-random-map-generation), [`IonRed`](/keys/ionred/#scope-random-map-generation), [`IonGreen`](/keys/iongreen/#scope-random-map-generation), [`IonBlue`](/keys/ionblue/#scope-random-map-generation), [`IonGround`](/keys/ionground/#scope-random-map-generation) and [`IonLevel`](/keys/ionlevel/#scope-random-map-generation) from its `[Lighting]` section, along with its trigger types and tag types. [Ion storms](/systems/ion-storms/#random-maps) covers what those triggers do.

This pass also decides, with one chance in four, whether a river on this map may have a waterfall.

:::caution[An ion storm leaves a snow map brighter than it started]
A tundra or taiga map is lit at three quarters of the hour's ambient level. The level an ion storm restores when it ends is the hour's full level. On a snow map with `UseIonStorms=yes`, the first storm to end therefore leaves the map brighter than it began.
:::

## The passes in order

The passes run in the order below. Each reads only the settings listed beside it. No pass reads [`Description`](/keys/description/#scope-random-map-generation).

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

Many passes and branches run only under a condition, and most of those conditions are the biome:

| Pass or branch | Runs only when |
| --- | --- |
| The water pass | [`WaterAmount`](/keys/wateramount/) is not `0` |
| The river | `WaterAmount` is above `20`, and the biome is not desert |
| The arctic lake and river, which lay ice | The biome is tundra; taiga uses the ordinary ones |
| Ice smoothing | The biome is tundra |
| Region splitting, the source of a generated map's cliffs | The biome is **not** tundra |
| Floodlights | `Time` is dusk or night, or `UseTransitions=yes` |
| Veinholes | The theater is not arctic, so never on tundra or taiga |
| Creatures in a tiberium field | The Firestorm addon is present, and the field is not a start-point field |
| Urban areas | The biome is temperate, desert or mutated |
| Rural settlements | The biome is tundra or taiga |
| Mold and crystal growths | The biome is mutated |
| Scattered rock overlays | The biome is temperate, desert or mutated |
| The hill height walk | [`Ruggedness`](/keys/ruggedness/) is at least `2` |

## Water

[`WaterAmount`](/keys/wateramount/) sets a budget of water cells, not a share of the map. The budget is `WaterAmount` times the playable area times a factor for the biome, plus 100 cells. The river and the lake each spend from that budget.

The pass tries to lay one river and then one lake, making up to ten attempts at each. On tundra both are arctic variants that lay ice instead of open water, and the ice smoothing pass then blends the ice into the ground around it. Taiga is also a snow map, but it gets the ordinary river and lake and no ice.

Open water uses tiles from the theater's [`WaterSet`](/keys/waterset/). On the mutated biome, up to two stretches of the lake are turned into swamp. An ordinary river can include one waterfall, but only on a map where the one-in-four roll allowed waterfalls. A tundra river never has one.

## Regions, cliffs and ramps

A **region** is a group of connected cells at one ground height. Regions decide where a generated map has cliffs, ramps and bridges. The pass builds them in three steps:

1. Each body of water becomes a region and grows four cells outward into the ground around it.
2. The remaining ground is divided into land regions.
3. Any dry region larger than the limit [`RegionSize`](/keys/regionsize/) sets is split in two, and one half moves to a new height. Splitting repeats until no dry region is over the limit. A higher `RegionSize` allows larger regions, so the map has fewer height changes.

Nothing places a cliff directly. Cliffs form where the regions from step 3 meet at different heights. Tundra skips step 3, so `RegionSize` has no effect there.

The pass then connects neighboring regions:

- A water region gets bridges between the dry regions on its banks, but only where both banks stand at the water's height.
- Where two dry regions meet at different heights, the generator tries to carve one ramp between them. With a chance of about [`Accessibility`](/keys/accessibility/) percent, rolled once per pair, it tries for one or two more. A pair can get fewer ramps than requested when no suitable spot is found.

Ramp and cliff artwork comes from the theater. Tiles from the cliff roles, including [`CliffRamps`](/keys/clifframps/), count as rock face, and the generator does not build over them.

## Start points and the layout spread

The start points are chosen from the finished terrain, in the same step as the tiberium field sites:

1. The generator gathers fifteen candidate cells per player. It takes them from the large regions that can reach one another over ground, or from anywhere on the map when no region qualifies. Each candidate must be at least four cells inside the playable area, with a ten-by-ten block around it where a paved road could be laid.
2. From the candidates it picks a spread of cells as far apart as possible. Cells in different regions count as farther apart than they are, so the picks favor separate pieces of ground.
3. The first [`NumPlayers`](/keys/numplayers/#scope-random-map-generation) cells of the spread become the start points. The rest become the tiberium field sites. [`TiberiumLayout`](/keys/tiberiumlayout/) sets how long the spread is, and so how many field sites remain.

Each start point then claims the 400 nearest clear cells as **protected** ground. Protected ground gets no tiberium, no veinhole and no tree, and no ground-cover patch starts on it. The settlement passes avoid it, and hills do not raise or lower it.

Protected ground can still receive some ground cover. A patch that starts nearby can spread onto it, and scattered rough-ground tiles and rock overlays can land on it.

If any start point cannot reach 400 clear cells, the generator discards all the start points and picks again from the next values in the random sequence. The terrain stays as it is. Nothing limits the number of retries.

## Floodlights, veinholes and tiberium

Three placement passes follow, in this order.

### Floodlights

The floodlight pass rings each start point with `GALITE` structures, eight cells out. [`Time`](/keys/time/) sets how many: none in the morning or afternoon, two at dusk and four at night. [`UseTransitions=yes`](/keys/usetransitions/) always gives four and attaches the hour file's `Light On/Off` tag to each. A ring is placed only where every light in it can stand. The generator tries 21 ring angles before it leaves a start point unlit.

### Veinholes

The veinhole pass tries to place [`VeinholeMonsters`](/keys/veinholemonsters/) monsters. It makes at most 200 attempts in total, so a crowded map gets fewer. Each attempt picks a random clear cell and accepts it only when all of these hold, tested in this order:

1. The five-by-five block centered on the cell, plus a one-cell margin around it, lies within the playable area.
2. **All of:**
   - no cell of the block is protected ground;
   - every cell of the block is clear ground at the center's height, with no overlay.
3. The [veinhole placement test](/systems/veins/#placement) accepts the cell.

An accepted cell takes the `VEINHOLE` overlay, and its eight neighbors take `VEINHOLEDUMMY`. Veins are placed on the outer ring of the five-by-five block.

### Tiberium

The tiberium pass grows a field at every layout site, then one at each start point.

The layout fields share an amount of tiberium set by [`Tiberium`](/keys/tiberium/#scope-random-map-generation) and `NumPlayers`. It is divided evenly among them, with a small random variation per field, so more fields means smaller ones.

Each player's start field has a size of 500, plus 15 for every cell the player sits farther from the layout sites, on average, than the closest player does. The player farthest from the map's tiberium gets the largest start field, and the closest player gets 500. `Tiberium` does not change start fields.

A start field's site is the start point, inside the player's protected ground. Protected ground takes no tiberium, so the field grows from the edge of that ground.

A field grows outward from its site over clear ground in the playable area. It covers bare cells and thickens tiberium already there.

If growth runs out of room before the field reaches its size, the field starts again from its site. It gets at most nine attempts in all, and a field still short after the last one stays short.

Each field grows one of two tiberium overlay sets:

- With [`UseBlueTiberium=no`](/keys/usebluetiberium/), every field grows the first set.
- With `UseBlueTiberium=yes`, each layout field is rolled separately, with about a 70 percent chance of the first set. All start fields share one roll, with about a 75 percent chance of the first set, so every player starts beside the same kind.

A start field that grows the first set also gets a tiberium tree at its first cell. A layout field that grows the first set has about a one-in-four chance of one.

:::caution[At the dialog's setting, a field gets no creatures or a steady stream]
The map generator dialog's check box writes `TiberiumWildlife=30`. At that figure, each layout field's creature budget comes out as zero in three draws out of five. In the other two it is effectively unlimited, because of an arithmetic error that [`TiberiumWildlife`](/keys/tiberiumwildlife/) explains. A field with the unlimited budget keeps releasing creatures for as long as it grows. On a map built at the dialog's setting, about two layout fields in five have creatures and the rest have none.
:::

## Settlements

[`UrbanPresence`](/keys/urbanpresence/) places a different kind of settlement on each group of biomes:

- **Tundra and taiga** get rural settlements. Each is a dirt road junction on open, unprotected ground, with a few civilian buildings along its roads and a few civilian vehicles.
- **Temperate, desert and mutated** get urban areas. Each is a paved district grown outward from a cell, with roads, buildings, traffic and a pavement edge.

Either pass stops after ten attempts, however many settlements were asked for. The `UrbanPresence` page explains what that ceiling does to the setting's useful range.

Road pieces come from the theater. Rural roads use pieces counted from [`DirtRoadCurve`](/keys/dirtroadcurve/), and each network starts on a junction from [`DirtRoadJunction`](/keys/dirtroadjunction/). Urban roads use pieces counted from [`PavedRoads`](/keys/pavedroads/), and each run ends with a piece from [`PavedRoadEnds`](/keys/pavedroadends/). Neither pass ever selects a ramp piece, so [`DirtRoadSlopes`](/keys/dirtroadslopes/) and [`PavedRoadSlopes`](/keys/pavedroadslopes/) have no effect, and a generated road stops at a slope.

:::note[Temperate, desert and mutated maps get no rural settlements]
The urban pass contains a branch that would place a rural settlement, with a chance of one minus `UrbanPresence`. The setting is a whole number from `0` to `100`, so that chance is zero or less at every value from `1` up, and at `0` the pass places nothing. These three biomes therefore only ever receive urban areas.
:::

## Hills and ground cover

### Hills

Hills are raised after the tiberium, settlements and roads are in place. Those features keep their height, and the ground around them rises and falls. A cell keeps its height if any of these holds:

- it holds an overlay, such as tiberium or veins;
- something stands on it, such as a building, a tree or a creature;
- it is protected ground;
- its tile comes from a set without [`Morphable=yes`](/keys/morphable/).

Before the hills are worked out, one clear cell beside each shore tile and each cliff tile is also protected.

Each remaining cell then gets a height offset drawn from the offsets of its neighbors, which makes the ground roll instead of jump. Offsets range from two levels down to two levels up. [`Ruggedness`](/keys/ruggedness/) sets both the size of each step and how far the offsets may wander. The terrain is then raised or lowered one level at a time to meet the offsets, with slopes cut in as it changes. Below `Ruggedness=2` this walk is skipped, and the terrain keeps the heights the region pass gave it.

### Ground cover

Ground cover is placed last, and the two biome groups are handled differently. Both grow patches of tiles and woods, each from a single cell.

A patch or wood starts only on a cell that is clear ground, has no ramp and is not protected. On temperate, desert and mutated maps, that cell must also have no overlay and nothing standing on it.

From that cell, a patch spreads over neighboring clear ground with no ramp, no overlay and nothing standing on it. Protected ground does not stop it. A wood also spreads, but never onto protected ground.

On **temperate, desert and mutated** maps:

1. A mutated map first gets mold and crystal growths. Their numbers are drawn from ranges that grow with `Width` and `Height`.
2. A desert or mutated map gets 5 to 30 rough-ground tiles scattered across it.
3. Each cell where a patch may start is rolled for green ground, rough ground, sand and woods, in that order. The first roll that succeeds decides what the cell gets, and the rest are skipped. [`Vegetation`](/keys/vegetation/) scales only the green ground and woods chances. Temperate and mutated maps have no sand chance, and desert maps have no green ground chance.
4. Transition tiles are fixed up, and rock overlays are scattered across the map. A temperate map gets half as many rocks.

On **tundra and taiga** maps:

1. 5 to 30 rough-ground tiles are scattered across the map.
2. Each cell where a patch may start is rolled separately for a rough patch, a wood and a patch of rock tiles, so one cell can start more than one. `Vegetation` scales only the woods chance, and there is no green ground.
3. Transition tiles are fixed up. These maps get no rock overlays.

## What the theater must supply

The generator takes all its terrain artwork from the tile set roles a theater control file declares. The table groups those roles by the pass that needs them, so a modder adding or renumbering tile sets can see which part of generation a missing role would affect. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a tile index.

| Pass | Roles it reads |
| --- | --- |
| Water and ice | [`WaterSet`](/keys/waterset/), the shore, waterfall and swamp sets, [`Ice1Set`](/keys/ice1set/), [`Ice2Set`](/keys/ice2set/), [`Ice3Set`](/keys/ice3set/), [`IceShoreSet`](/keys/iceshoreset/) |
| Regions, cliffs and ramps | The cliff, slope and ramp sets, [`CliffRamps`](/keys/clifframps/) among them |
| Settlements | [`PavedRoads`](/keys/pavedroads/), [`PavedRoadEnds`](/keys/pavedroadends/), [`DirtRoadCurve`](/keys/dirtroadcurve/), [`DirtRoadJunction`](/keys/dirtroadjunction/), and the pavement and median sets |
| Ground cover | The green, rough, sand, rock, mold and crystal sets |

When a map is read, the engine discards the artwork of every tile type that no cell uses yet. A generated map places most of its tiles after that, so a tile set the generator needs must set [`RequiredForRMG=yes`](/keys/requiredforrmg/) to keep its artwork. The exemption applies only to a generated map: every map the dialog builds, and a scenario whose filename ends `.SED` or that sets `RandomMap=yes`.

## Handing the map over

When the last pass finishes, the generator starts vein growth and tiberium growth and spread, computes the radar image and returns the finished map to the scenario load.

The multiplayer setup runs only after that, because the start points do not exist until generation ends. Each house's starting units are created at the generated start points, [crates](/systems/crates/) are scattered if the match enables them, and computer players receive their credit multipliers. From then on, a generated map behaves as an ordinary scenario.

Before any of this, the multiplayer and skirmish lobbies check that the chosen scenario has a start point for every player by counting its `[Waypoints]` entries. A seed file has none, because its start points do not exist until the map is generated. For a file with no waypoints, the lobbies read [`NumPlayers`](/keys/numplayers/#scope-random-map-generation-2) from its `[RandomMap]` section instead.
