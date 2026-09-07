---
key: AIWallDefense
scope: side
label: Side wall defense count
see_also: [AIWallDefenseCoefficient, AIWallTowers, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes GDIWallDefense as each rules file sets it; any other side uses 0.
---

```ini title="rules.ini"
[GDI]
AIWallDefense=6
```

The fixed part of the cap on wall tower and base defense pairs a computer house playing for this side appends after planning its perimeter wall; [Walls and gates](/systems/ai-base-building/#walls-and-gates) gives the full expression. The cap binds only once the wall is long enough to ask for more pairs than it allows, and only a side with an [`AIWallTowers`](/keys/aiwalltowers/#scope-side) entry the house's country may own appends any.
