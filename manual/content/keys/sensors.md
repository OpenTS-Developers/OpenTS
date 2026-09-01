---
key: Sensors
summary: Whether the object reveals a nearby hidden object belonging to a house its own owner does not consider allied.
see_also: [SensorArray, "system:cloaking"]
when_omitted:
  kind: context-dependent
  note: "An InfantryType section starts at yes. An AircraftType, BuildingType or UnitType section starts at no."
---

The flag is read in two proximity tests, both of which look at the eight cells around something and neither of which marks a cell as sensed. A hidden vehicle, infantryman or aircraft arriving at the center of a cell is uncloaked when one of the neighboring cells inside the playable area holds a flagged object whose owner does not consider it allied. A cloaked structure is uncloaked, and refused a new cloak, while a flagged object whose owner does not consider the structure allied stands anywhere within one cell of its footprint.

Both tests use the detector owner's alliance list. The hidden object's owner may consider the detector allied without preventing detection.

Detection therefore costs the detector nothing and grants its house nothing at a distance: it marks nothing on the radar and lets nothing shoot at something still hidden elsewhere. That is what a [`SensorArray=yes`](/keys/sensorarray/) structure does instead, and the two mechanisms share no state.

The `SENSORS` [veteran ability](/systems/veterancy/#abilities) stands in for the flag in the first test only. A promoted detector reveals a passing vehicle and does nothing at all to a cloaked structure.

:::caution[Every InfantryType is a detector unless told otherwise]
InfantryTypes start with the flag already set, so a civilian, an engineer and a common rifleman all reveal hidden objects that move past them. Suppressing that takes an explicit `Sensors=no` in the section.
:::
