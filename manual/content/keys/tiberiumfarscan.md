---
key: TiberiumFarScan
summary: Distance a harvester searches for a Tiberium patch when it has none in mind.
see_also: ["system:tiberium", "TiberiumNearScan"]
when_omitted:
  kind: value
  value: "32"
---

The value is given in cells, with fractions accepted, and sets the limit of the outward ring search, so the default reaches 31 cells from the harvester. The outermost ring searched sits one cell inside the value, after the value is truncated to whole cells. It applies when a harvester sets out with no recorded patch to return to; failing that search is what leaves the harvester idle and marks its house short of Tiberium. The same figure bounds a [weeder's](/systems/veins/) search for veins.
