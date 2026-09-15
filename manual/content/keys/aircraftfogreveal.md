---
key: AircraftFogReveal
summary: Radius in cells that an airborne aircraft with no sight of its own clears of fog.
see_also: ["system:map-visibility", Sight]
when_omitted:
  kind: value
  value: "6"
---

The value is reached only by an aircraft whose type sets [`Sight=0`](/keys/sight/). That aircraft must also be off the ground, and fog of war must be on. No other aircraft reads the setting.

The pass it drives is not an ordinary reveal. It lifts fog from cells that are already out from under the shroud and leaves shrouded cells alone. An aircraft of this kind therefore clears the way ahead of it without ever uncovering new map. The height test that blocks a reveal behind high ground applies only while the aircraft is below half the `[General]` [`FlightLevel`](/keys/flightlevel/), the rules-wide figure rather than the type's own key of that name. Above that height it clears every cell in the radius.
