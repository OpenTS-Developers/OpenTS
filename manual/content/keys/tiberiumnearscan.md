---
key: TiberiumNearScan
summary: Distance a harvester searches for more Tiberium once it is already working a field.
see_also: ["system:tiberium", "TiberiumFarScan"]
when_omitted:
  kind: value
  value: "6"
---

The value is given in cells, with fractions accepted, and sets the limit of the outward ring search, so the default reaches 5 cells from the harvester. The outermost ring searched sits one cell inside the value, after the value is truncated to whole cells. It applies after a cell runs out: a harvester with room left hops to the nearest patch inside it, and a full one records that patch as the place to come back to. The same figure bounds a [weeder's](/systems/veins/) search for veins.
