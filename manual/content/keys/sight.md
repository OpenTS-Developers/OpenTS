---
key: Sight
summary: Radius in cells that the type reveals around itself for its owner.
see_also: ["system:map-visibility", VeteranSight, EliteSight]
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

A vehicle, infantryman or structure with no sight range reveals nothing. For them, `Sight=0` leaves no range unless the type gives that rank a positive [`VeteranSight`](/keys/veteransight/#scope-aircrafttype) or [`EliteSight`](/keys/elitesight/). Aircraft differ: a landed aircraft always reveals a radius of one cell, and an airborne aircraft with no sight range lifts only the fog, within [`AircraftFogReveal`](/keys/aircraftfogreveal/) cells, while fog of war is on. [Who looks, and when](/systems/map-visibility/#who-looks-and-when) covers the aircraft cases in full.

`Sight=` is the base range. Rookies and below-rookie objects use it; a type can replace it for promoted objects, as [Sight range](/systems/map-visibility/#sight-range) describes. Every look reaches at most ten cells.

Out of weapon range, a human player's object fights back only when the attacker is within its sight range plus half a cell, which includes the range for its rank and the height and ability bonuses. [Retaliation](/systems/target-selection/#retaliation) gives the rule.
