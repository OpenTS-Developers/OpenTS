---
key: TrailerAnim
scope: voxelanimtype
label: Voxel debris trail
see_also: ["AttachedSystem", "BounceAnim", "ExpireAnim"]
when_omitted:
  kind: value
  value: none
---

An animation of the named type is created every other frame for as long as the piece is alive. The frames are the even frames of the game clock, not a count of the piece's own. A plume that has to stay attached to the piece is better served by [`AttachedSystem`](/keys/attachedsystem/), which ties one particle system to it for the whole flight.

:::caution[The trail is placed far above the piece]
The trail takes the piece's horizontal position unchanged. Its height, though, is put through the conversion that turns a screen height into a world height, the opposite of the one that would place it correctly. The trail is drawn about nine times as high above the ground as the piece shedding it, and the gap grows as the piece climbs.
:::
