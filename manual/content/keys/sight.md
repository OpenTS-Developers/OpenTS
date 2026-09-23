---
key: Sight
summary: Radius in cells that the type reveals around itself for its owner.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "0"
---

The value is a plain count of cells. Unlike [`GuardRange=`](/keys/guardrange/) and the other distance settings on the same section, or a weapon's [`Range=`](/keys/range/), it is not converted from cells to leptons when it is read, and the code that needs a lepton figure multiplies it itself.

At zero the type reveals nothing. An aircraft type is the exception: `Sight=0` is what sends it down the [`AircraftFogReveal`](/keys/aircraftfogreveal/) path, which lifts fog without touching the shroud.

The stored figure is only the starting point. [Sight range](/systems/map-visibility/#sight-range) covers the height bonus and the veteran multiplier applied over it, and the cap of ten cells that the scan applies to the result.

```ini title="rules.ini"
[MYSCOUT] ; example UnitType
Sight=8
MoveToShroud=yes
```
