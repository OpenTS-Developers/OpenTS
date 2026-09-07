---
key: AIBuildsWalls
scope: side
label: Side wall planning
see_also: [AIBaseDefensesWithWalls, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The second side takes NodAIBuildsWalls as each rules file sets it; every other side builds walls.
---

```ini title="rules.ini"
[Nod]
AIBuildsWalls=no
```

Whether a computer house playing for this side closes its base plan with the node that runs [the perimeter wall planner](/systems/ai-base-building/#walls-and-gates). The global [`AIBuildsWalls`](/keys/aibuildswalls/#scope-global-rules) must allow walls as well. A side that will not build a wall receives the extra base-defense placeholders instead, as does one that sets [`AIBaseDefensesWithWalls=yes`](/keys/aibasedefenseswithwalls/#scope-side).
