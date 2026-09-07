---
key: ScrapExplosion
summary: The animations a destroyed object leaves behind while scrap wreckage is switched on.
see_also: [Explosion, ScrapMetal, Explodes, DebrisTypes, "system:destruction-and-debris"]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
Explosion=TWLT070,FRAG1,FRAG3      ; AnimTypes registered in [Animations]
ScrapExplosion=FBALL1_SCRAP,FRAGG1_SCRAP
```

This is a second [`Explosion`](/keys/explosion/) list, spent in place of the first while [`ScrapMetal`](/keys/scrapmetal/) is on. It is spent exactly as that key describes — one entry per wreck for a vehicle or aircraft, one per cell of the footprint for a structure, the last entry for a vehicle that explodes while it still has ammunition — and an InfantryType stores it and never reads it back.

A type that leaves this empty keeps its ordinary animations even with the switch on, so a ruleset can convert its arsenal a few entries at a time.

Both lists are read whichever way the switch is set, and which one is spent is settled as the object dies rather than when the rules are read, so one set of rules serves a match played either way.
