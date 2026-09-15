---
key: ThreatAvoidanceCoefficient
summary: How heavily the type weighs the region threat figures when the pathfinder prices a route.
see_also: ["system:base-attacked", AvoidThreats, ThreatPosed]
when_omitted:
  kind: value
  value: "0"
---

The pathfinder multiplies [the threat figure](/systems/base-attacked/#what-reads-the-map) of the region a cell falls in by this value. A diagonal shortcut is refused once the product reaches `1`. On a two-leg straight line, each cell whose product reaches `0.01` counts as threatened; the line is abandoned once more than three have accumulated, or at the first one for an object that is not fearless. The hierarchical route planner prices each subzone step by the same product. Every one of those tests runs inside the route search, so the figure is read when a route is planned, never while the object follows the route it chose. It plays no part in target scoring or in the base defense response, which read [`ThreatPosed`](/keys/threatposed/) instead.

The figure is copied from the type onto each instance as it is placed on the map. A team's [`AvoidThreats=yes`](/keys/avoidthreats/) overrides that copy with `1` for as long as the object is on the team.

:::caution[At zero the pathfinder's threat tests can never fire]
The product is zero for every cell while the coefficient is zero, however high the region figures climb, so no shortcut is refused and no cell is counted as threatened. Every type starts at zero, which leaves `AvoidThreats=yes` the only way threat avoidance reaches the pathfinder until a coefficient is written.
:::
