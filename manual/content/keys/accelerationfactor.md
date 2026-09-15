---
key: AccelerationFactor
summary: The step a vehicle's throttle climbs by each frame while it is speeding up.
see_also: ["Accelerates", "DeaccelerationFactor"]
when_omitted:
  kind: value
  value: ".03"
---

A vehicle's throttle is a fraction of its own top speed, running from 0 to 1. This figure is added to the throttle once each time the vehicle's movement is stepped. A frame that crosses into a new cell adds it twice. The addition continues while the throttle is below the speed it has been told to hold. At the stock figure a vehicle needs 34 frames, a little over two seconds, to work up from a standstill to full speed. Only a vehicle with [`Accelerates=yes`](/keys/accelerates/) and moved by the drive locomotor is affected.

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
AccelerationFactor=.06 ; reaches full speed in about 17 frames, half the stock ramp-up
```

:::caution[The braking figure is on a different scale]
This figure is added to the throttle directly, while [`DeaccelerationFactor`](/keys/deaccelerationfactor/) is multiplied by the vehicle's top speed before it is subtracted. The two numbers are not comparable. Writing the same value in both does not give a vehicle symmetrical acceleration and braking.
:::
