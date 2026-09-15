---
key: RailgunDamageRadius
summary: How far an object's center may stand from a railgun beam and still be caught by it.
see_also: ["IsRailgun", "AmbientDamage", "CollapseChance"]
when_omitted:
  kind: value
  value: "128"
---

The distance is written in leptons, 256 to a cell, and is measured square to the beam from its line to the object's center. Two kinds of victim are taken without being measured at all: a structure standing in a cell the beam crosses, and the object the shot was aimed at. [`IsRailgun`](/keys/israilgun/) covers the walk that gathers them and the damage they take.

```ini title="rules.ini"
[CombatDamage]
RailgunDamageRadius=256   ; catches centers up to one cell from the beam line
```

Because the measurement is to a center point rather than to a hull, widening it catches units standing beside the beam rather than making the beam itself thicker.
