---
key: AIWallDefenseCoefficient
scope: side
label: Side wall defense coefficient
see_also: [AIWallDefense, AIWallTowers, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes GDIWallDefenseCoefficient as each rules file sets it; any other side uses 0.
---

```ini title="rules.ini"
[GDI]
AIWallDefenseCoefficient=3
```

The difficulty-scaled part of that cap: it is multiplied by `3` minus the house's difficulty slot and the product is added to [`AIWallDefense`](/keys/aiwalldefense/#scope-side), so an advantaged computer house rings its wall with more defenses than a penalized one. [Walls and gates](/systems/ai-base-building/#walls-and-gates) gives the full expression.
