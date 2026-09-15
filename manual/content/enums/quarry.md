---
enum_id: QuarryType
slug: quarry
title: Quarry target category
summary: Broad target categories a team attack mission scans for, and that a house records as its preferred target.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [quarry]
source_files: [code/quarry.hh, code/quarry.cpp, code/team.cpp]
values:
  - { constant: QUARRY_NONE, value: 0, input: "0", meaning: "No category, and no target is assigned." }
  - { constant: QUARRY_ANYTHING, value: 1, input: "1", meaning: "Any suitable enemy." }
  - { constant: QUARRY_BUILDINGS, value: 2, input: "2", meaning: "Buildings in general." }
  - { constant: QUARRY_HARVESTERS, value: 3, input: "3", meaning: "Harvesters or refineries." }
  - { constant: QUARRY_INFANTRY, value: 4, input: "4", meaning: "Infantry." }
  - { constant: QUARRY_VEHICLES, value: 5, input: "5", meaning: "Combat vehicles." }
  - { constant: QUARRY_FACTORIES, value: 6, input: "6", meaning: "Production factories." }
  - { constant: QUARRY_DEFENSE, value: 7, input: "7", meaning: "Base-defense buildings." }
  - { constant: QUARRY_THREAT, value: 8, input: "8", meaning: "Runs the identical scan to category 1; nothing measures distance to a base." }
  - { constant: QUARRY_POWER, value: 9, input: "9", meaning: "Power facilities." }
---

A **quarry** is the kind of object a team is hunting rather than a single target it has picked. The category filters a scan over the whole map. A team's attack mission passes it to the team leader, and [target selection](/systems/target-selection/) scores the candidates it admits. Category 2 admits every building the scan can see, including the power facilities, production factories and base-defense buildings that categories 6, 7 and 9 single out. Those three narrow the scan to their own kind of building. A value outside the list behaves exactly as category 0 does: no target is assigned, and the team moves on to the next mission in its script.

The categories have a second consumer that runs no scan. [Preferred target...](/mapping/actions/taction-preferred-target/) stores one on a house instead of passing it to a team, and that page covers what becomes of it.
