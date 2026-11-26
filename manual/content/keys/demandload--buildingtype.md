---
key: DemandLoad
scope: buildingtype
label: Structure shape
see_also: ["DemandLoadBuildup", "Image", "Theater"]
when_omitted:
  kind: value
  value: "no"
---

With `DemandLoad=yes`, a structure type's main shape is loaded when a scenario starts or a saved game is loaded, rather than with the rules, and stays loaded until the next scenario or saved game is loaded. The file is the [main-shape filename](/keys/image/#scope-buildingtype), resolved for the scenario theater, and can be loose or in any archive. A shape the game cannot find is not drawn.

The flag covers only the main shape. The construction animation has its own setting, [`DemandLoadBuildup`](/keys/demandloadbuildup/). The deploying, door, under-door, bib and special Z overlay shapes are always loaded with the rules.
