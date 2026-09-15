---
key: Crusher
summary: Lets a vehicle drive over crushable objects and walls instead of being blocked by them.
see_also: ["AutoCrush", "TiltsWhenCrushes", "Crushable", "SpeedType"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle with the flag treats a cell holding a [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) object as passable rather than as something to shoot or route around. So does one whose crew has earned the crusher ability from [`VeteranAbilities`](/keys/veteranabilities/). An unarmed vehicle needs the flag to act on enemy infantry at all: with it, a move order onto them ends in the vehicle driving over them. It is also what lets the pathfinder count a friendly crushable wall as destroyable rather than solid. Driving onto a crushable wall overlay destroys the segment outright, ignoring ownership and damage stage; [Crushing, clearing and selling](/systems/walls-and-gates/#crushing-clearing-and-selling) covers what that does to the wall.

The flag also has one effect that has nothing to do with crushing. A UnitType whose [terrain speed class](/reference/enums/speed-type/) no file has settled yet is given `Track` when the flag is set and `Wheel` when it is not. A [`SpeedType=`](/keys/speedtype/) in the same section then overrides that. Leaving both keys out therefore costs a crusher against the tracked movement table and a non-crusher against the wheeled one.

Two further settings sit on top of it. [`Crush=`](/keys/crush/) sets how close a target must be before a crusher drives over it instead of firing. The rules-wide [`AutoCrush=`](/keys/autocrush/#scope-global-rules) sets the computer-house IQ below which a vehicle stops answering attacks by running the attacker over; the per-type [`AutoCrush`](/keys/autocrush/#scope-aircrafttype) flag reaches no live branch. [`TiltsWhenCrushes`](/keys/tiltswhencrushes/) decides whether the hull lurches while it does.
