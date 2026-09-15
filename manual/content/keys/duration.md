---
key: Duration
summary: How many frames a voxel animation lives before it performs its impact and is removed.
see_also: ["IsMeteor", "Elasticity", "ExpireAnim"]
when_omitted:
  kind: value
  value: "30"
---

A count of game frames, fifteen to the second. The counter is loaded when the piece is created and stepped down once per frame. Reaching zero runs the impact, then deletes the piece. The impact is [`ExpireAnim`](/keys/expireanim/#scope-voxelanimtype), the blast, [`ExpireSound`](/keys/expiresound/#scope-voxelanimtype), and whatever cratering or Tiberium seeding the type calls for.

The full count is rarely spent. Debris that comes to rest before the counter runs out has it cleared, and the impact follows on the next frame. A value of `0` or below expires the piece on its first frame of logic, before it has moved at all.

A meteor spends the count differently. Its counter is shortened by a random 0 to 19 frames at creation, and the shortened figure is also the flight time the piece is placed back along. It is the length of the approach as much as the length of the life. [`IsMeteor`](/keys/ismeteor/#scope-voxelanimtype) describes that flight.
