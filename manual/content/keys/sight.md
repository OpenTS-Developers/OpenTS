---
key: Sight
summary: Radius in cells that the type reveals around itself for its owner.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[MYSCOUT] ; example UnitType
Sight=8
MoveToShroud=yes
```

`Sight` counts whole cells. Unlike [`GuardRange`](/keys/guardrange/) and a weapon's [`Range`](/keys/range/#scope-weapontype), it is not converted to leptons.

A vehicle, infantryman or structure with `Sight=0` reveals nothing. Aircraft differ: a landed aircraft always reveals a radius of one cell, and an airborne aircraft with `Sight=0` lifts only the fog, within [`AircraftFogReveal`](/keys/aircraftfogreveal/) cells, while fog of war is on. [Who looks, and when](/systems/map-visibility/#who-looks-and-when) covers the aircraft cases in full.

This value is the starting range. For a vehicle, infantryman or structure, [Sight range](/systems/map-visibility/#sight-range) adds the height and veteran bonuses.

A human player's object also uses `Sight` when it is attacked from beyond its weapon's range: it fights back only if the attacker is within `Sight` plus half a cell. [Retaliation](/systems/target-selection/#retaliation) gives the full rule.
