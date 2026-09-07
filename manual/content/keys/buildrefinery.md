---
key: BuildRefinery
summary: The refineries a computer house plans and measures its income against, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The refinery enters [the base plan](/systems/ai-base-building/#building-the-plan) through the ordinary candidate scan; the first entry this house may own only marks which queued refinery receives the plan's extra copies, and a refinery the scan never queued receives none. The money logic counts every listed type as a refinery, both when judging whether a computer house can still earn and while it sells its base back to recover, and prices the first entry the country the house acts as may own, or entry 0 when it may own none. The list also decides a crate reward — a crate that would hand out a unit gives a free harvester instead when the house that opened it owns a building of any listed type and no harvester, a human house included, unless [`UnitCrateType`](/keys/unitcratetype/) names a unit and overrides the pick.

An empty list leaves the money logic judging that the house can still earn, and no crate hands out a harvester.
