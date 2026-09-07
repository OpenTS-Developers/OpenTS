---
key: AIBaseDefensesWithWalls
scope: side
label: Side defenses beside walls
see_also: [AIBaseDefensePlaceholders, AIBuildsWalls, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: yes for the second side and no for every other.
---

```ini title="rules.ini"
[Nod]
AIBaseDefensesWithWalls=yes
```

Whether a computer house playing for this side appends its extra base-defense placeholders even when it also builds a wall. A side that sets `no` gets them only when it will not wall itself in, because its wall planner lays defenses along the wall instead; the second side gets them either way, as the shipped rules always did for Nod.
