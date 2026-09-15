---
key: ThreatPosed
summary: How much danger the type represents, used by team weighting, the region threat map, and base-defense call-ups.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

No part of target scoring reads this figure; it is what an object is worth as a danger, not what it is worth as a target. Three systems consume it:

- The weight an object has when a team tallies its members. Separately, this figure plus the type's [`Points`](/keys/points/) (and the same sum for anything carried as cargo) forms an object's worth to the computer. The computer's superweapon targeting reads it: nine times in ten it picks the enemy building with the greatest worth.
- The per-house, per-region threat map, credited when the object is placed on the map and debited when it leaves. The pathfinder reads that map through [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/).
- The size of a computer house's response when its base is attacked: the attacker's figure multiplied by [`ComputerBaseDefenseResponse`](/keys/computerbasedefenseresponse/) is the defensive strength the house tries to assemble.

```ini title="rules.ini"
[MYTANK] ; example UnitType
ThreatPosed=25
```

A type left at `0` contributes nothing to any of the three, so an attacker of that type provokes no base-defense call-up at all.
