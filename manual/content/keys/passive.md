---
key: Passive
summary: Marks a vehicle as holding a fixed route at a fixed pace, which is how the cars behind a locomotive move.
see_also: ["IsTrain", "Accelerates", "MovementRestrictedTo"]
when_omitted:
  kind: value
  value: "no"
---

The flag changes three things about how a vehicle is driven, and nothing about what it is or what it may be ordered to do.

Its speed is left out of the ramping. Even with [`Accelerates=yes`](/keys/accelerates/) set, the step that walks a driving vehicle's throttle toward the speed of the ground under it never runs for a passive one. It is neither brought up to speed nor braked as it nears its destination.

Its route must reach the destination outright. When the search arrives at the destination cell and finds it impassable, an ordinary vehicle settles for the route it has and stops beside it. A passive vehicle takes no such offer and keeps searching, so a blocked destination leaves it with no route at all rather than a shortened one.

It may change track in mid-move. At a junction where the next cell can be entered, an ordinary vehicle holds the track it is on. A passive vehicle switches to the new turn track there and then, which is what lets a car follow the vehicle ahead of it around a corner.

```ini title="rules.ini"
[MYORECAR] ; a UnitType registered in [VehicleTypes]
Passive=yes
IsTrain=yes
MovementRestrictedTo=Railroad
```
