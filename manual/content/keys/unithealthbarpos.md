---
key: UnitHealthBarPos
summary: Where a vehicle's or aircraft's row of health pips is drawn.
see_also: [InfantryHealthBarPos]
when_omitted:
  kind: value
  value: "-15,-25"
---

`UnitHealthBarPos=` sets where a vehicle's or aircraft's row of health pips begins. The value is a horizontal and a vertical offset in pixels; positive values move the row right and down. [Health bar positions](/formats/ui-ini/#health-bar-positions) covers the point the offset is measured from and how the row is laid out.

A vehicle with [`IsCoreDefender=yes`](/keys/iscoredefender/#scope-unittype) draws a structure's pip bar along the near edge of its footprint instead and does not use the key.

```ini title="UI.INI"
[Ingame]
UnitHealthBarPos=-15,-20
```
