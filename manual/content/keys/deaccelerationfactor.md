---
key: DeaccelerationFactor
summary: The share of a vehicle's top speed its throttle drops by each frame while it is slowing down.
see_also: ["Accelerates", "AccelerationFactor", "SlowdownDistance"]
when_omitted:
  kind: value
  value: ".002"
---

The figure is multiplied by the vehicle's top speed, and the product is subtracted from its throttle once per frame. That happens both while it is braking over the last [`SlowdownDistance`](/keys/slowdowndistance/) of its journey and whenever it is running faster than the speed it has been told to hold. Braking on the approach never takes the throttle below three tenths; only a stop order brings a vehicle to rest. Only a vehicle with [`Accelerates=yes`](/keys/accelerates/) and moved by the drive locomotor is affected.

Multiplying by top speed is what makes the two stock figures line up. A vehicle at [`Speed=6`](/keys/speed/#scope-aircrafttype) sheds `.03` of its throttle per frame, exactly what [`AccelerationFactor`](/keys/accelerationfactor/) adds. A faster type brakes proportionally harder and a slower one more gently from the same written figure.
