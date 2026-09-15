---
enum_id: MZoneType
slug: movement-zone
title: Movement zone
summary: Pathfinding classes, each deciding which terrain and obstacles a type of that class may cross.
representation: token
bindings:
  key_value_types: [mzonetype]
  scripting_parameter_types: []
source_files: [code/mzone.hh, code/ccini.cpp, code/map.cpp, code/cell.cpp]
values:
  - { constant: MZONE_NORMAL, value: 0, input: "Normal", meaning: "Terrestrial movement to which every crushable and every blocked cell is closed." }
  - { constant: MZONE_CRUSHER, value: 1, input: "Crusher", meaning: "Terrestrial movement to which a cell held by a crushable overlay is open." }
  - { constant: MZONE_DESTROYER, value: 2, input: "Destroyer", meaning: "Terrestrial movement to which a cell held by a crushable overlay, a wall, or a fully blocking terrain object is open." }
  - { constant: MZONE_AMPHIBIOUS_DESTROYER, value: 3, input: "AmphibiousDestroyer", meaning: "Destroyer movement extended across water as well." }
  - { constant: MZONE_AMPHIBIOUS_CRUSHER, value: 4, input: "AmphibiousCrusher", meaning: "Crusher movement extended across water as well." }
  - { constant: MZONE_AMPHIBIOUS, value: 5, input: "Amphibious", meaning: "Movement across land and water." }
  - { constant: MZONE_SUBTERANNEAN, value: 6, input: "Subterannean", meaning: "Subterranean movement.", note: "The accepted token preserves the engine's historical spelling." }
  - { constant: MZONE_INFANTRY, value: 7, input: "Infantry", meaning: "Infantry movement." }
  - { constant: MZONE_INFANTRY_DESTROYER, value: 8, input: "InfantryDestroyer", meaning: "Infantry movement to which crushable, wall and fully blocking terrain cells are open as well." }
  - { constant: MZONE_FLYER, value: 9, input: "Fly", meaning: "Air movement, to which every cell inside the playable area is open." }
---

The names say which cells join a class's [movement zones](/glossary/#movement-zone), not what an object of that class may do to them. A class decides only which cells count as crossable while the zones are built and a route is plotted. `Crusher` opens a cell held by a crushable overlay; the three destroyer classes also open a wall or a terrain object that fills every one of its standing places. `Destroyer` therefore crosses fully blocking trees and rocks as readily as it crosses walls. Whether a vehicle actually flattens a wall it drives into is [`Crusher=yes`](/keys/crusher/), which is read separately and can be set on a type of any class.

What each class is compared against is a single blockage rating held for the cell, not its [land type](/reference/enums/land-type/). [`TemperateOccupationBits`](/keys/temperateoccupationbits/) covers how a terrain object's occupation figure decides how much of its cell the object blocks, without changing the terrain at all. The ten classes are fixed by the engine, and each one costs the map a zone table of its own. [`MovementZone`](/keys/movementzone/) covers what naming one does to a type.
