---
enum_id: LandType
slug: land-type
title: Land type
summary: Terrain classes used by movement, pathfinding, and terrain restrictions.
representation: token
bindings:
  key_value_types: [landtype]
  scripting_parameter_types: []
source_files: [code/land.hh, code/const.cpp, code/cell.cpp, code/isotype.cpp]
values:
  - { constant: LAND_CLEAR, value: 0, input: "Clear", meaning: "Clear ground." }
  - { constant: LAND_ROAD, value: 1, input: "Road", meaning: "Road surface." }
  - { constant: LAND_WATER, value: 2, input: "Water", meaning: "Open water." }
  - { constant: LAND_ROCK, value: 3, input: "Rock", meaning: "Impassable rock." }
  - { constant: LAND_WALL, value: 4, input: "Wall", meaning: "Wall terrain." }
  - { constant: LAND_TIBERIUM, value: 5, input: "Tiberium", meaning: "Tiberium field." }
  - { constant: LAND_BEACH, value: 6, input: "Beach", meaning: "Beach terrain." }
  - { constant: LAND_ROUGH, value: 7, input: "Rough", meaning: "Rough ground." }
  - { constant: LAND_ICE, value: 8, input: "Ice", meaning: "Solid ice." }
  - { constant: LAND_RAILROAD, value: 9, input: "Railroad", meaning: "Rail track terrain." }
  - { constant: LAND_TUNNEL, value: 10, input: "Tunnel", meaning: "Tunnel terrain." }
  - { constant: LAND_WEEDS, value: 11, input: "Weeds", meaning: "Vein weed terrain." }
---

A cell's land type is derived rather than authored: it comes from the tile drawn beneath the cell, and a tile can only produce nine of these twelve. `Wall`, `Tiberium` and `Weeds` reach a cell through an overlay standing on it, which [`Land`](/keys/land/) covers. [`CliffBackImpassability`](/keys/cliffbackimpassability/) can override both. At `2`, a cell standing a full cliff step below any of the six neighbors that setting examines is rewritten to `Rock` at the end of the same pass. Only four of the twelve are open to that rewrite on the ordinary path: `Clear`, `Water`, `Beach` and `Ice`. A cell whose tile already reports `Road`, `Rough` or `Railroad` therefore survives the cliff rewrite unchanged.

Each token also names the rules section that holds that terrain's movement costs. Those sections are `[Clear]`, `[Road]`, `[Water]` and the rest, and the twelve classes are what [speed type](/reference/enums/speed-type/) is costed against. The set is fixed by the engine: a mod can retune a section but cannot add a thirteenth terrain class or a section for one.
