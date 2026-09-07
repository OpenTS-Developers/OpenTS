---
key: AIBaseDefensePlaceholders
scope: side
label: Side defense placeholders
see_also: [AIBaseDefensesWithWalls, AIBaseDefenseCoefficient, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: 3 for the first side and 2 for every other.
---

```ini title="rules.ini"
[GDI]
AIBaseDefensePlaceholders=3
```

How many extra `-1` base-defense placeholders per difficulty step a computer house playing for this side appends to its plan when it will not build a wall, or whenever [`AIBaseDefensesWithWalls=yes`](/keys/aibasedefenseswithwalls/#scope-side); the count is `(3 - Difficulty)` times this figure, each placeholder preceded by the side's [`AIWallTowers`](/keys/aiwalltowers/#scope-side) entry when the country may own one. [Building the plan](/systems/ai-base-building/#building-the-plan) shows where they land.
