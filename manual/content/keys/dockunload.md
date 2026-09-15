---
key: DockUnload
summary: Lets a Tiberium harvester dock at a building and unload there.
see_also: ["system:tiberium", "Dock", "Refinery"]
when_omitted:
  kind: value
  value: "no"
---

This is the flag a building answers a harvester's docking request with, and it is the only one that admits a Tiberium harvester: [`Refinery=yes`](/keys/refinery/) supplies the dock point and the unloading animations but grants no access on its own. The harvester's house must have declared the building's house an ally, and only that direction is checked, so a one-way alliance admits the request. A [weed refinery](/systems/veins/) is the exception: it admits a weeder on [`Weeder=yes`](/keys/weeder/) alone, without this flag. A building already holding an attached object refuses the request until that object leaves.
