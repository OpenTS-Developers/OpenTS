---
key: AIBuildsWalls
scope: global-rules
label: Global wall planning
see_also: [NodAIBuildsWalls, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="rules.ini"
[General]
AIBuildsWalls=yes
```

Lets a computer house close its base plan with the node that runs [the perimeter wall planner](/systems/ai-base-building/#walls-and-gates). With `no` no house appends that node, so no computer base reaches the planner, and every house instead receives the extra base-defense placeholders. Each side can refuse walls for itself with its own [`AIBuildsWalls`](/keys/aibuildswalls/#scope-side).
