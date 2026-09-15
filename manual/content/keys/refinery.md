---
key: Refinery
summary: Marks a building as a Tiberium refinery for its dock point and unloading animations.
see_also: ["system:tiberium", "DockUnload", "Storage"]
when_omitted:
  kind: value
  value: "no"
---

The flag moves the building's dock point half a cell east of its center. It plays the [`ProductionAnim`](/keys/productionanim/) animation once a docked harvester has emptied itself, and keeps an idle harvester from parking on the building by sending it to a cell beside it.

Admitting a harvester in the first place is [`DockUnload=yes`](/keys/dockunload/), and storing what it brings is [`Storage`](/keys/storage/); neither follows from this flag.
