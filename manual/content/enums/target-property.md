---
enum_id: TargetPropertyType
slug: target-property
title: Target property
summary: Selection rules used when a team script picks one building from all those of a named type.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [target-property]
source_files: [code/target.hh, code/team.cpp, code/map.cpp]
values:
  - { constant: TPROPERTY_LEAST_THREAT, value: 0, input: "0", meaning: "Pick the candidate standing in the map region the team's house rates as least threatened." }
  - { constant: TPROPERTY_GREATEST_THREAT, value: 1, input: "1", meaning: "Pick the candidate standing in the map region the team's house rates as most threatened." }
  - { constant: TPROPERTY_NEAREST, value: 2, input: "2", meaning: "Pick the candidate nearest the team's first member." }
  - { constant: TPROPERTY_FARTHEST, value: 3, input: "3", meaning: "Pick the candidate farthest from the team's first member." }
---

Neither threat property weighs the building itself. Each house keeps its [threat map](/systems/base-attacked/) as one figure per **map region**, and a region is a block of four by four cells. Both properties read the figure the team member's own house keeps for the region a candidate stands in. `Least threat` heads for the quietest part of the map holding a building of that type, and `Greatest threat` for the busiest.

That figure is neither confined to the region it is filed under nor a measure of hostile force alone. An object raising it writes across a three-by-three block of regions. Its full **risk**, which is its type's [`ThreatPosed`](/keys/threatposed/), goes into the region it stands in. Half of that goes into the four regions beside it, and a quarter into the four corners, so every reading is smeared across a neighborhood.

One test settles which houses receive an object's figures. It runs as the object is placed, as it crosses from one region into another, as its owner changes and as it is taken off the map. The table gives the four positions a house can hold toward the object's owner. Only one kind of ally is spared: an alliance protects a human house from its ally's risk and does nothing for a computer one.

| The house keeping the figure | Is the object's risk added to it? |
| --- | --- |
| The object's own owner | No |
| A human house allied to the owner | No |
| A computer house allied to the owner | Yes |
| Any house not allied to the owner | Yes |

When a house makes or breaks an alliance it rebuilds its threat map. That rebuild applies the test to infantry, vehicles and aircraft alone. Every other object, structures included, is added with no ownership or alliance test at all, so from that point on a house's own structures raise its own figures.

A value outside the four scores every candidate identically, so none is selected. Whenever the search leaves the team with no target, it moves on to the next line of its script.
