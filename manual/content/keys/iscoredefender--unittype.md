---
key: IsCoreDefender
scope: unittype
label: Core defender vehicle
see_also: [SensorArray, "system:emp-pulse"]
when_omitted:
  kind: value
  value: "no"
---

Two unrelated things read the flag on a vehicle.

[An EM pulse](/systems/emp-pulse/#what-a-pulse-reaches) does not paralyze it. Every other vehicle inside the blast has its locomotor powered off, is stopped where it stands and is stunned for the pulse's duration; a flagged one keeps moving and only springs its [Paralyzed](/mapping/events/tevent-paralyzed/) trigger event.

It is also drawn the way a structure is. A selected one takes the three-dimensional selection box and the pip bar laid along its near edge. The drawing also appears, unselected, while the vehicle stands below the surface on a cell a [sensor array](/keys/sensorarray/) has marked. The test is one of depth, so it catches a vehicle burrowing past the array and never one driving under a bridge or one that is cloaked. Every other vehicle gets a bracket and a row of health pips. The box is built on a vertical extent of 700 leptons rather than the 200 an ordinary vehicle has.
