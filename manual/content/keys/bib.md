---
key: Bib
summary: Opens the eastern edge of the structure's footprint to vehicles.
see_also: [BibShape, Foundation, Harvester, Refinery, Weeder]
when_omitted:
  kind: value
  value: "no"
---

One test reads the flag, and it runs when a vehicle checks whether it may enter a cell. A flagged structure standing in that cell stops blocking it whenever the cell one step east is not also part of the same structure (for a rectangular foundation, its whole eastern column). Vehicles drive onto that column as though the structure were not there; infantry are unaffected, since their own entry test never looks at the flag.

Two narrower carve-outs open one more cell, on foundations wide enough that the cell two east and one south of the structure's own cell is not already in that eastern column:

- a [`Refinery=yes`](/keys/refinery/) structure stops blocking an allied vehicle that sets [`Harvester=yes`](/keys/harvester/#scope-unittype);
- a [`Weeder=yes`](/keys/weeder/#scope-buildingtype) structure does the same for an allied vehicle that sets [`Weeder=yes`](/keys/weeder/#scope-unittype).

Both read the vehicle's own flag and the structure's, not what the vehicle was ordered to do, so any allied harvester passes whether or not it is heading for the dock.

:::caution[The flag neither draws the apron nor enlarges the footprint]
The apron artwork is [`BibShape`](/keys/bibshape/) in the art file, which is drawn whenever it resolves and never reads this setting; a structure can set either one without the other. The footprint is likewise unchanged: every reader of a structure's height asks for it without the apron, so the cells a structure occupies are exactly the ones [`Foundation`](/keys/foundation/) gives it.
:::
