---
key: IsVehicleTransport
summary: Lets units board this transport, not just infantry.
see_also: [Passengers, Size, SizeLimit, "system:transports"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYAPC] ; a UnitType registered in [VehicleTypes]
Passengers=5
SizeLimit=3
IsVehicleTransport=yes
```

A transport refuses a unit outright unless this is set, and refuses it the same way it refuses a type that carries nobody at all — the cursor does not change and no enter order is offered. Infantry are unaffected either way.

The refusal is answered by the transport itself rather than decided at the cursor, so it holds for every route into a hold: a player's order, a computer team loading up, and the final check as the passenger reaches the transport.

With the flag set and nothing else changed, a unit occupies one slot exactly as an infantryman does. [`Size`](/keys/size/) and [`SizeLimit`](/keys/sizelimit/) are what make a vehicle cost more than a rifleman.

A transport that carries units can itself be carried if some other transport admits it. Nothing forbids the arrangement; [`SizeLimit`](/keys/sizelimit/) is what a ruleset uses to rule it out.
