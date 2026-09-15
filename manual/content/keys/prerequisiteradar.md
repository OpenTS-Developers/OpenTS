---
key: PrerequisiteRadar
summary: The BuildingTypes that satisfy a RADAR prerequisite.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

Owning at least one live structure of any type on this list satisfies a `RADAR` entry in a [`Prerequisite=`](/keys/prerequisite/) list. The entries are alternatives, not requirements: the first one the house owns ends the search, and order has no other meaning.

The computer's [base planner](/systems/ai-base-building/#building-the-plan) does not read this list. It answers a `RADAR` entry from its own build lists instead.

The list is read as BuildingType IDs without regard to case, and a name matching none is dropped. An empty list leaves `RADAR` impossible to satisfy, which blocks every type naming it from a player's [production](/systems/production/).
