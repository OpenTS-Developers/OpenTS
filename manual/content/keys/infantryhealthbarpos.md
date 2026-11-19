---
key: InfantryHealthBarPos
summary: Where an infantry object's row of health pips is drawn.
see_also: [UnitHealthBarPos]
when_omitted:
  kind: value
  value: "-5,-24"
---

`InfantryHealthBarPos=` sets where an infantry object's row of health pips begins. The value is a horizontal and a vertical offset in pixels; positive values move the row right and down. [Health bar positions](/formats/ui-ini/#health-bar-positions) covers the point the offset is measured from and how the row is laid out.

```ini title="UI.INI"
[Ingame]
InfantryHealthBarPos=-5,-20
```
