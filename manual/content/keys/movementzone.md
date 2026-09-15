---
key: MovementZone
summary: The terrain connectivity class the type's route and reachability tests are answered in.
see_also: ["system:base-attacked"]
when_omitted:
  kind: value
  value: "Normal"
---

For each [movement zone](/reference/enums/movement-zone/) class, the map is divided into connected zones, and every question of the form "can this object get from here to there" is answered against the zone map its type names. The pathfinder uses the same class to decide which terrain a route may cross.

`Subterannean` has one extra consequence: a type that names it is marked as subterranean, which is what puts it on the tunneling path rather than on the surface.

Two automatic decisions compare zones directly rather than plotting a route.

- The [base defense call-up](/systems/base-attacked/#which-objects-qualify) drops a candidate whose destination is not in the same zone as the damaged object's.
- [Target selection](/systems/target-selection/#why-a-candidate-is-rejected) rejects a candidate in a different zone under **All of:** the scan is a ring or full-map scan, the scanner is not a structure, and the scanner is not an aircraft.

Both use the class named by the type doing the moving or looking, never the other type's. A value the engine does not recognize resolves to no zone at all rather than falling back to `Normal`.
