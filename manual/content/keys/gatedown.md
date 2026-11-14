---
key: GateDown
summary: The default sound played at a gate as its door starts going down.
see_also: ["system:walls-and-gates", "Gate"]
when_omitted:
  kind: value
  value: none
---

The sound plays once, at the gate's position, when its door starts going down. It does not play when a gate going up shifts to going down again, or when the gate is already going down or down.

This sound is only played if the gate did not set [`GateDownSound`](/keys/gatedownsound/). If it did, it would play its specific sound instead.
