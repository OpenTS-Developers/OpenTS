---
key: Passengers
summary: How much passenger space the type can carry at once.
see_also: [Size, SizeLimit, IsVehicleTransport, PipScale, CrewEscape, Crewed, VisibleLoad, AIIonCannonAPCValue, "system:transports"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[MYAPC] ; a UnitType registered in [VehicleTypes]
Passengers=5
PipScale=Passengers
```

The figure is a budget of space rather than a head count, spent by each passenger's [`Size`](/keys/size/). Since that defaults to one, a ruleset that never mentions sizes gets the head count it always had: five passengers are five objects. Give a passenger a larger size and it spends more of the same budget.

Zero is what makes a type not a transport at all, and that is checked before anything else — a load request to a type with no capacity is refused outright, and so is one from a house the transport is not allied with. A unit is refused as well unless [`IsVehicleTransport`](/keys/isvehicletransport/) is set. Past those, the request has to fit: the passenger's [`Size`](/keys/size/) within the transport's [`SizeLimit`](/keys/sizelimit/), and the space already spent plus that size within this figure. A vehicle additionally refuses to load while it is standing on a water or shore cell.

Any capacity above zero changes several unrelated things about the type:

- Its pip row switches to showing the hold, one pip per unit of space colored by each passenger's own [`Pip`](/keys/pip/), instead of a scaled quantity. The row still has no pips until [`PipScale`](/keys/pipscale/) gives it a length, and that length is whatever [`PipScale`](/keys/pipscale/) gives — five under the passenger scale however large the capacity, ten under `Power`, eight under `Charge`.
- A vehicle gains the ability to deploy — unloading is a deploy — where a vehicle with no capacity, no [`DeploysInto`](/keys/deploysinto/) and no [`IsMobileEMP=yes`](/keys/ismobileemp/) has nothing to deploy into and is refused. That unload is then blocked while the vehicle stands on a bridge, and blocked again when its own cell or any of the four cells around it lies under one.
- A vehicle stops producing an escaping crew when it dies, whatever [`Crewed=yes`](/keys/crewed/) says; [`CrewEscape`](/keys/crewescape/) covers that exclusion.
- A computer house treats the type as a transport: teams look for it when a script wants one, and an enemy ion cannon rates it at [`AIIonCannonAPCValue`](/keys/aiioncannonapcvalue/) rather than as an ordinary vehicle.
