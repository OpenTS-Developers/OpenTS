---
key: Crew
summary: The InfantryType that ordinarily leaves a destroyed or sold object as its survivor.
see_also: ["system:capture", Crewed, Technician]
when_omitted:
  kind: value
  value: none
---

Each survivor picks its type afresh, and two settings are read ahead of this one. A structure that has never been captured and whose own production kind is buildings offers the `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) type on a one-in-four roll. [`Technician`](/keys/technician/) then replaces the type named here for an object whose house belongs to no side, and on a 15% roll for an armed object whose house has one. Everything else uses this type. Only a [`Crewed=yes`](/keys/crewed/) structure or vehicle produces survivors at all; [Survivors](/systems/capture/#survivors) covers the count and the per-cell odds.

A value naming an InfantryType that no `[InfantryTypes]` entry registers is not refused. The read registers the name itself, and because object sections are read after `[General]`, a section written under that name is still picked up in the same pass. Such a type is missing from the list, not from the game. A name with no section of its own registers a type with nothing but engine defaults: no artwork and no animation sequence data, and the per-frame animation logic reads that sequence table without checking.

:::danger[A `Crewed=yes` structure cannot be sold until this is set]
The value starts as no type at all, and the survivor loop a sale runs reads each pick before testing it for nothing. Selling such a structure therefore ends the game on the first survivor the engineer roll does not supply.
:::
