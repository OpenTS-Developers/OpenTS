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

:::danger[Name a type here, or selling a crewed structure can crash]
With no type named by this key or by the owner's side, selling a structure crashes the game whenever one of its survivors would be of this type. That happens on 15% of the survivors of an armed `Crewed=yes` structure, and on every survivor of one whose own country has no side. A destroyed structure skips that survivor instead. The stock rules name `CTECH`.
:::
