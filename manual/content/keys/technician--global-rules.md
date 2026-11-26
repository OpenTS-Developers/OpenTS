---
key: Technician
scope: global-rules
label: Survivor technician type
see_also: ["system:capture", Crew, Engineer, Nominal]
when_omitted:
  kind: value
  value: none
---

A [`Crewed=yes`](/keys/crewed/) structure or vehicle takes a technician as its survivor in either of two cases:

- its owning house's country has no [side](/keys/side/#scope-housetype), so every survivor is a technician;
- that country has a side and the object has a primary weapon, so each survivor is a technician on a 15% roll.

This key is the type of every such survivor. [Survivor types](/systems/capture/#survivor-types) gives the whole order, including the engineer a structure can leave first. Every country in the stock rules has a side, so the first case arises only for a country that a mod leaves without one.

A side can name its own technician type for the objects of houses acting as its countries. [`Technician`](/keys/technician/#scope-side) covers that scope.

If neither this key nor the owner's side names a technician type, the object skips any survivor that would use it. The stock rules name `CTECH`.
