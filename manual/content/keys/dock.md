---
key: Dock
summary: BuildingTypes an object returns to for docking.
see_also: ["system:tiberium", "DockUnload", "Harvester"]
when_omitted:
  kind: value
  value: "none"
---

A harvester keeps its harvest mission only while its house owns at least one building of a listed type; with none owned, or with an empty list, it is switched to guard. When a load is ready it asks each listed type in turn for a bay among its own house's buildings and takes the first type that answers. List order therefore decides which kind of building is tried first. Allied buildings are not considered.
