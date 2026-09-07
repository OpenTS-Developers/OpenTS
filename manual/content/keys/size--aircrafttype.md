---
key: Size
scope: aircrafttype
label: Passenger size
see_also: [SizeLimit, IsVehicleTransport, Passengers, PipScale, "system:transports"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
Size=3
```

The figure is what this object costs a transport that carries it. A transport accepts it only if the figure is within the transport's [`SizeLimit`](/keys/sizelimit/), and only if adding it to what is already aboard stays within the transport's [`Passengers`](/keys/passengers/). So a `Size=3` passenger fills three of a five-slot hold and leaves room for two more at the default size.

The default of one makes the hold a plain head count, which is what a ruleset that never mentions sizes gets: five passengers are five objects whatever they are.

The figure also drives the hold's pip row — a passenger claims as many pips as it claims space, so the `Size=3` passenger above shows three. [`PipScale`](/keys/pipscale/) covers the row itself.

Size is read on anything built from a TechnoType, but it only ever means anything for an object that can become a passenger. Nothing can order an aircraft into a transport, and a building is never a passenger, so on those two the key is inert.
