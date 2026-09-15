---
key: TechLevel
scope: house-per-scenario
label: House tech level
see_also: ["system:production"]
when_omitted:
  kind: computed
  note: The scenario number stands in, so a house in the third mission of a campaign starts at level 3.
---

This is the level every object type's own [`TechLevel=`](/keys/techlevel/#scope-aircrafttype) is compared against for this house.

Every playing house a non-campaign session sets up has this value overwritten with the level chosen for that session; the Neutral and Special houses keep their default. The assignment takes effect in campaign games only.
