---
key: Engineer
scope: global-rules
label: Survivor engineer type
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: none
---

An uncaptured structure whose type sets [`Factory=BuildingType`](/keys/factory/) has a one-in-four chance of picking this InfantryType for each survivor. The ordinary crew or technician type serves on the other rolls. The structure loses the engineer roll once it [changes hands](/systems/capture/#what-changes-hands).

The roll applies whether the structure is destroyed or sold. A sale follows the [survivor type limits](/systems/capture/#survivor-types) and produces no survivors if the structure names an [`UndeploysInto`](/keys/undeploysinto/) vehicle. The shipped construction yard names one.

A side can name its own engineer type for structures of houses acting as its countries. [`Engineer`](/keys/engineer/#scope-side) covers that scope.

If neither this key nor the owner's side names an engineer type, the structure skips any survivor that would use it.
