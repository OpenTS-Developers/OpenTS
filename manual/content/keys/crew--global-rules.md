---
key: Crew
scope: global-rules
label: Survivor type
see_also: ["system:capture", Crewed, CrewEscape, Technician, Engineer]
when_omitted:
  kind: value
  value: none
---

Only a [`Crewed=yes`](/keys/crewed/) structure or vehicle produces survivors. [`CrewEscape`](/keys/crewescape/) covers a vehicle's crew and [survivors](/systems/capture/#survivors) the count a structure releases. This key is the type of every survivor that the [survivor type order](/systems/capture/#survivor-types) does not give to the engineer or the technician.

The value does not have to be listed under `[InfantryTypes]`. A name the game does not already know registers a new InfantryType, and a section of that name is still read, because object sections are read after `[General]`. A name with no section of its own gives a type with only engine defaults and no animation sequence, and a survivor of that type crashes the game once it animates.

A side can name its own crew type for the objects of houses acting as its countries. [`Crew`](/keys/crew/#scope-side) covers that scope.

If neither this key nor the owner's side names a crew type, the object skips any survivor that would use it.
