---
key: CollapseChance
summary: Percent chance that a destroyable cliff comes down when something reaches its cell.
see_also: ["IsRailgun", "IsSonic"]
when_omitted:
  kind: value
  value: "100"
---

The roll is made only for a cell the theater marks as a destroyable cliff. Any of these sets the roll going:

- an explosion, wherever it goes off;
- the point at which an [`IsRailgun=yes`](/keys/israilgun/) beam is stopped by rising ground;
- the cell that beam finally settles on;
- any cell an [`IsSonic=yes`](/keys/issonic/) wave has swept across, once on every frame the wave lives. The wave damages its whole accumulated swathe again each frame, so a cliff under a sonic beam is rolled for many times over.

The figure is a percentage: `100` brings the cliff down on every hit, `0` on none, and anything above `100` behaves as `100`. Under a sonic beam the figure barely matters: any value above zero brings the face down almost at once, since every cliff cell the wave has reached is rolled for again on every frame the wave lives.
