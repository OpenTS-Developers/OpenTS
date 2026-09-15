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

This is a second [`Explosion`](/keys/explosion/) list, spent in place of the first while [`ScrapMetal`](/keys/scrapmetal/) is on. It is spent exactly as that key describes, with the entry picked at random from the list: one animation per wreck for a vehicle or aircraft, one per cell of the footprint for a structure. The exception is a vehicle flagged [`Explodes=yes`](/keys/explodes/) or holding the `EXPLODES` [veteran ability](/systems/veterancy/#abilities): while it still holds ammunition, or has no ammunition limit at all, it spends the list's last entry rather than a random one. An InfantryType stores it and never reads it back.

A type that leaves this empty keeps its ordinary animations even with the switch on, so a ruleset can convert its arsenal a few entries at a time.

Both lists are read whichever way the switch is set. Which one is spent is settled as the object dies rather than when the rules are read, so one set of rules serves a match played either way.
