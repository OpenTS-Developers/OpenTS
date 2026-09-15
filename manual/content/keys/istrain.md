---
key: IsTrain
summary: Marks a vehicle as rolling stock, which changes how it finds a path, what blocks it and when it may leave the map.
when_omitted:
  kind: value
  value: "no"
---

The flag turns a vehicle into one car of a train and changes how it moves, how it is routed, and how other things treat it.

Pathfinding gives it no long-range search: the coarse pass that ordinary vehicles use to plan across the map is switched off for it. The step-by-step search is narrowed as well: the three cells in the rear arc, those more than a quarter turn off the direction the vehicle is currently facing, are struck out before the search begins. It cannot start a journey by backing up or turning sharply.

Blockages barely stop it. Every obstacle short of strictly prohibited terrain is treated as a clear cell when it decides whether it may enter one. It does not ask an allied object standing in its way to move aside, and it does not stop short and settle for a nearby spot the way an ordinary vehicle does. An unarmed one counts a cell held by enemy infantry as destroyable rather than impassable. It never scatters, whatever is coming at it.

Leaving the map is permitted. An ordinary object that is not a loaner may not leave, but a train may, and it is deleted once it is gone. The deletion holds off while the order it is following still points at a cell inside the playable area.

Stopping unhooks the cars behind it: a car that is not itself following another stops the whole chain trailing it, and that chain is broken again when one of them is destroyed.

A vehicle thief will not take a train. It is passed over as a target and answers the thief's click with a plain selection.
