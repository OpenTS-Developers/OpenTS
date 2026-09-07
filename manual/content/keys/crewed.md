---
key: Crewed
summary: Allows the type to put a surviving infantry on the map when it is destroyed.
see_also: ["system:capture", CrewEscape, Passengers, "system:transports"]
when_omitted:
  kind: value
  value: "no"
---

Only a BuildingType's and a UnitType's value is consulted. A structure without it produces no survivors at all, whatever [`SurvivorRate`](/keys/survivorrate/) and [`SurvivorDivisor`](/keys/survivordivisor/) work out to; [Survivors](/systems/capture/#survivors) covers the count and the per-cell odds. A vehicle without it produces no escaping crew, and a vehicle that carries passengers is refused the crew as well.

An InfantryType and an AircraftType read the value and store it, but no path reads it back for either.

A vehicle stolen by an infantryman — by either theft feature — is the exception on the vehicle side: [the hijacker steps back out](/systems/capture/#stealing-a-vehicle) when that vehicle dies whether or not the type is crewed, and without the escape roll a crew would have to pass.
