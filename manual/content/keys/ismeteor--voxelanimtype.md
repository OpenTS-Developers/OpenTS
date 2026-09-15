---
key: IsMeteor
scope: voxelanimtype
label: Voxel meteor flight
see_also: ["MinZVel", "MaxZVel", "MaxXYVel", "Spawns", "SpawnCount", "CraterLevel", "Duration"]
when_omitted:
  kind: value
  value: "no"
---

Ordinary debris is created where it was asked for and thrown up from there. A meteor is created at the far end of a flight instead: a velocity is drawn, and the lifetime is shortened by a random 0 to 19 frames. The piece is placed that many frames back along the velocity, so it travels in and reaches the coordinate it was created for as the lifetime runs out.

The velocity itself is drawn differently in three ways.

- Its vertical component is [`MinZVel`](/keys/minzvel/#scope-voxelanimtype) exactly, with no random pick and no reference to [`MaxZVel`](/keys/maxzvel/). The sign decides the approach: a negative figure brings the meteor down onto the target from above, a positive one brings it up from underneath.
- Its two horizontal components come from the [`MaxXYVel`](/keys/maxxyvel/#scope-voxelanimtype) range and are then mirrored together whenever `X` falls below `-Y`. The mirror keeps the flight direction in the downward half of the screen, so a meteor always comes in from the upper half of the view and never from the bottom edge.
- Gravity is added back to the piece each frame after the physics have taken it away, so a meteor travels a straight line where other debris arcs.

Coming down on solid ground adds two effects ordinary debris never gets: the terrain is deformed around the impact cell according to [`CraterLevel`](/keys/craterlevel/), and [`Spawns`](/keys/spawns/#scope-voxelanimtype) breaks the piece into fresh debris. An end low over water substitutes the last animation in the rules' splash list for the wake and small splash other debris makes there.
