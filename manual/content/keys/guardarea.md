---
key: GuardArea
summary: The IQ level at which a computer house leaves idle objects in Area Guard instead of Guard.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "4"
---

A house whose [`IQ`](/keys/iq/) is below this level puts idle objects into plain Guard, which scans only out to the object's guard radius and holds position. At or above it an object takes Area Guard instead, which scans out to twice the guard range from the spot it was left at and pursues what it finds. When the guard range is unset, the scan falls back to twice the longer weapon range.

Only the infantry path tests for a weapon. An infantry unit takes Area Guard for having one, an unarmed engineer and an unarmed vehicle thief take it as well, and any other unarmed soldier takes plain Guard. A vehicle takes Area Guard whatever it is armed with. A member of a team always takes plain Guard.

A vehicle holding the `GUARD_AREA` ability takes Area Guard whatever the level says. For infantry the test is computer-only, but the vehicle path has no such gate: a human house whose map-given `IQ` reaches this level sends its idle vehicles into Area Guard too.
