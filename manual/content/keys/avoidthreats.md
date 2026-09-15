---
key: AvoidThreats
summary: Fixes every member's threat avoidance at `1` while it is on the team.
see_also: ["system:base-attacked", ThreatAvoidanceCoefficient]
when_omitted:
  kind: value
  value: "no"
---

Each time a member asks how much weight to give [the region threat figures](/systems/base-attacked/#what-reads-the-map), the answer is a weight of `1` while its team's TeamType is marked this way, whatever [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) its own type sets. The weight multiplies each region's threat figure, so at `1` every figure counts at face value. The question is asked afresh on every route, so a member reverts to its type's own coefficient the moment it leaves the team.

At `1` the pathfinder refuses a diagonal shortcut out of any region with a threat figure of `1` or more. On a straight-line leg, a cell counts as threatened when its region has any threat figure above zero.
