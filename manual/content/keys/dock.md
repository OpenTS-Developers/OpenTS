---
key: Dock
summary: BuildingTypes an object returns to for docking.
see_also: ["system:tiberium", "DockUnload", "Harvester"]
when_omitted:
  kind: value
  value: "none"
---

A harvester keeps its harvest mission only while its house owns at least one building of a listed type. If the house owns none, or the list is empty, the harvester switches to guard. With a load to deliver, it heads for the nearest building of any listed type that its own house owns and that can take it; it never picks an allied building on its own. List order matters only between two equally distant buildings, where the earlier type wins. [Unloading](/systems/tiberium/#unloading) covers the preference for a primary building and when a harvester waits at a busy one instead.
