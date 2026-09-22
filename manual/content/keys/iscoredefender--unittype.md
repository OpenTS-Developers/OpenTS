---
key: IsCoreDefender
scope: unittype
label: Core defender vehicle
see_also: [SensorArray, "ImmuneToEMP", "system:emp-pulse"]
when_omitted:
  kind: value
  value: "no"
---

Two unrelated things read the flag on a vehicle.

When the type omits [`ImmuneToEMP`](/keys/immunetoemp/), the flag makes the vehicle immune to [EM pulses](/systems/emp-pulse/#what-a-pulse-reaches). A pulse that stops and stuns other vehicles leaves it moving and only springs its [Paralyzed](/mapping/events/tevent-paralyzed/) trigger event. `ImmuneToEMP=no` removes the immunity.

It is also drawn the way a structure is. A selected one takes the three-dimensional selection box and the pip bar laid along its near edge. The drawing also appears, unselected, while the vehicle stands below the surface on a cell a [sensor array](/keys/sensorarray/) has marked. The test is one of depth, so it catches a vehicle burrowing past the array and never one driving under a bridge or one that is cloaked. Every other vehicle gets a bracket and a row of health pips. The box is built on a vertical extent of 700 leptons rather than the 200 an ordinary vehicle has.
