---
key: Harvester
scope: unittype
label: Tiberium harvester
see_also: ["system:tiberium", "Storage", "Dock", "Weeder"]
when_omitted:
  kind: value
  value: "no"
---

The vehicle takes the harvest mission on its own, fills from cells of Tiberium up to its [`Storage`](/keys/storage/), and returns to a [`DockUnload=yes`](/keys/dockunload/) building drawn from its [`Dock`](/keys/dock/) list. [Harvesting](/systems/tiberium/#harvesting) covers the search, load, and unload cycle.

:::caution[The vein branch wins when both flags are set]
A type with both this flag and [`Weeder=yes`](/keys/weeder/#scope-unittype) waits for Tiberium ground the way a Tiberium harvester does, and then runs the vein branch on it. One or two units are added to the first Tiberium compartment per cycle, and the cell keeps every stage it had.
:::
