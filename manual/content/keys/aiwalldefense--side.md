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

The fixed part of the cap on the wall tower and base defense pairs a computer house playing for this side appends after planning its perimeter wall. Demand is `0.2` pairs per wall node laid, one pair per five wall cells, truncated to whole pairs and capped at `(3 - Difficulty)` times [`AIWallDefenseCoefficient`](/keys/aiwalldefensecoefficient/) plus this figure. The cap binds only once the wall is long enough to ask for more pairs than it allows. Only a side with an [`AIWallTowers`](/keys/aiwalltowers/) entry the house's country may own appends any; [walls and gates](/systems/ai-base-building/#walls-and-gates) covers the placement detail.
