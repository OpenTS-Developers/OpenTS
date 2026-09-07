---
key: Prerequisite
summary: The structures a house must own before the type appears on its build list.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

Every entry must be satisfied at once. An entry naming a BuildingType requires at least one live structure of exactly that type; the tally counts a structure from the moment it is placed rather than when its buildup finishes, and keeps counting one that has been switched off.

Seven reserved names stand for a group instead, each satisfied by owning anything on the matching rules list: `POWER`, `FACTORY`, `BARRACKS`, `RADAR`, `TECH`, `GDIFACTORY` and `NODFACTORY`. [Prerequisites](/systems/production/#what-a-house-may-build) maps them to their lists and covers the separate rule an entry naming an upgrade follows.

```ini title="rules.ini"
[MYTANK] ; example UnitType
Prerequisite=FACTORY,MYRADAR
; FACTORY: any structure on the PrerequisiteFactory list
; MYRADAR: example radar BuildingType, required by its own ID
```

Names are matched without regard to case; an entry matching neither a group name nor a BuildingType ID is dropped. A computer house is not put through this test when it produces — its [base planning](/systems/ai-base-building/) runs its own check over the same key when deciding what to place.
