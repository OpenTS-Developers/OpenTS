---
key: WarRatio
summary: Parsed war factory share of a computer base that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", WarLimit, BuildWeapons]
when_omitted:
  kind: value
  value: ".1"
---

Nothing weighs a computer base by proportion. A war factory reaches the plan because [the plan assembly](/systems/ai-base-building/#building-the-plan) moves the first `BuildWeapons` entry the country this house [acts as](/keys/actslike/) [may own](/keys/owner/) to the second slot of the candidate array, behind the barracks. The expansion pass appends every candidate whose own [`Prerequisite`](/keys/prerequisite/) list the queue already satisfies. Nothing reserves a share of the base for it.
