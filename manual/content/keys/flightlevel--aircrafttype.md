---
key: FlightLevel
scope: aircrafttype
label: Type cruising altitude
see_also: ["IsDropship", "SlowdownDistance"]
when_omitted:
  kind: value
  value: "-1"
  note: A stored -1 means the type has no altitude of its own, and every read falls back on the `[General]` figure.
---

The figure is a height above the ground in leptons, read as a raw count rather than in cells: the stock dropship's `FlightLevel=1600` is a little over six cells' worth of height.

```ini title="rules.ini"
[DSHP] ; the stock dropship, an AircraftType
FlightLevel=1600
```

Whenever the engine asks a type how high it flies, it hands back this figure unless the figure is `-1`, in which case it hands back the [rules-wide altitude](/keys/flightlevel/#scope-global-rules) instead. Writing `-1` explicitly is therefore the same as leaving the key out.

An aircraft is created at this height and climbs back to it on takeoff. Several of its missions test its height against the figure exactly to decide whether it is already airborne. The fly locomotor raises the level above the figure when high ground is coming up along the heading, and lowers it toward the target as a hunter seeker closes in. An [`IsDropship=yes`](/keys/isdropship/) type sinks to a third of the figure once it is within its [`SlowdownDistance`](/keys/slowdowndistance/).

The key is accepted in an AircraftType, BuildingType, InfantryType or UnitType section, but only the fly locomotor and the aircraft missions read it. On a type moved any other way it does nothing.
