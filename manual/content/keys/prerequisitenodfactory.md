---
key: PrerequisiteNodFactory
summary: The BuildingTypes that satisfy a NODFACTORY prerequisite.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

Owning at least one live structure of any type on this list satisfies a `NODFACTORY` entry in a [`Prerequisite=`](/keys/prerequisite/) list. The entries are alternatives, not requirements: the first one the house owns ends the search, and order carries no other meaning.

The computer's [base planner](/systems/ai-base-building/#building-the-plan) treats the group the same way: while the plan is assembled, any listed type already queued satisfies the entry.

The list is read as BuildingType IDs without regard to case, and a name matching none is dropped. An empty list leaves `NODFACTORY` impossible to satisfy, which blocks every type naming it, in the plan and in production alike.
