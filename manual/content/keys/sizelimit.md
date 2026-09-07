---
key: SizeLimit
summary: The largest single passenger a transport will accept.
see_also: [Size, IsVehicleTransport, Passengers, "system:transports"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[MYAPC] ; a UnitType registered in [VehicleTypes]
Passengers=5
SizeLimit=3
IsVehicleTransport=yes
```

A passenger whose [`Size`](/keys/size/) exceeds this figure is refused however much room is left, so the transport above turns away anything larger than three even while empty. The test is separate from the room test: a passenger has to pass both, and failing on size alone gives the same refusal as a full hold.

The default of one admits objects of the default size and nothing larger. A transport that is to carry something bigger has to say so, which is why raising [`Passengers`](/keys/passengers/) on its own is not enough to make room for a large passenger.

Together with [`IsVehicleTransport`](/keys/isvehicletransport/) this is how a ruleset decides which vehicles a transport takes: the flag decides whether vehicles may board at all, and this figure decides how large they may be.

A carryall is not restrained by either, because it takes hold of a vehicle directly rather than asking to load it. Giving vehicles a larger [`Size`](/keys/size/) therefore does not stop a carryall lifting them.

Where Red Alert 2 reads this as a floating-point number and defaults it to zero, OpenTS reads a whole number and defaults it to one. Zero would refuse every default-sized passenger, which would stop existing transports carrying infantry.
