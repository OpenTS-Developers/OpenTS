---
key: WalkRate
summary: The number of game frames between steps of a moving object's animation count.
when_omitted:
  kind: value
  value: "1"
---

An infantryman, a vehicle or an aircraft keeps a running count of animation steps taken. The count advances by one whenever the object's locomotor reports that it is moving and the game frame number divides evenly by this figure. Only a vehicle's artwork reads the resulting count. It indexes the walk frames of a shape-drawn vehicle and the phase of the harvesting animation. A voxel model's motion frames advance on every second step instead, so each of those frames holds for twice this figure. Infantry and aircraft animation is paced elsewhere, and a structure keeps no count at all.

A larger figure holds each step longer. The stock Titan's `WalkRate=2` advances its walk frame every second game frame, and the Core Defender's `WalkRate=4` every fourth. Because the test is against the game frame counter rather than a counter of the object's own, every object sharing a figure steps in unison.

:::danger[A figure of zero divides by zero]
The frame counter is divided by this figure once per frame for every moving object on the map, with no guard. Writing `WalkRate=0` therefore brings the game down the moment an object of that type starts to move, whether or not anything draws its walk cycle.
:::
