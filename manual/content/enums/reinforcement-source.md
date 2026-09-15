---
enum_id: SourceType
slug: reinforcement-source
title: Reinforcement source
summary: Map-edge origins used when objects cross a scenario's boundary.
representation: token
bindings:
  key_value_types: [sourcetype]
  scripting_parameter_types: []
source_files: [code/source.hh, code/_source.cpp, code/display.cpp, code/reinf.cpp]
values:
  - { constant: SOURCE_NORTH, value: 0, input: "North", meaning: "Enter from the north edge." }
  - { constant: SOURCE_EAST, value: 1, input: "East", meaning: "Enter from the east edge." }
  - { constant: SOURCE_SOUTH, value: 2, input: "South", meaning: "Enter from the south edge." }
  - { constant: SOURCE_WEST, value: 3, input: "West", meaning: "Enter from the west edge." }
  - { constant: SOURCE_AIR, value: 4, input: "Air", meaning: "Names no boundary; nothing in the engine reads it." }
---

Only four of the five name a boundary. `Air` names none: nothing in the engine reads it, so a house that stores it places reinforcements as though the edge were `North`. The routine that turns a source into a cell to scan along has a case for the four compass values alone. [`Edge`](/keys/edge/) covers what each route does with the source a house holds.
