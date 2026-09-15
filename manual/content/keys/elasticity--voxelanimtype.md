---
key: Elasticity
scope: voxelanimtype
label: Voxel debris bounciness
see_also: ["BounceAnim", "BounceSound", "Duration", "DamageRadius"]
when_omitted:
  kind: value
  value: "0.8"
---

The multiplier applied to the debris' velocity every time it rebounds. The velocity is rotated into the local space of the slope it struck, scaled by this figure, flipped vertically and rotated back out. The setting decides how much speed survives a contact, and nothing about the direction the piece leaves in.

Bouncing ends as soon as the piece has too little motion left to keep tracking. The combined figure is the length of the piece's velocity vector, with the height it is still standing off the ground multiplied by the fixed gravity factor of 1.4 and added to the vertical component. A result below `2.5` is reported as settling instead of as a strike. Settling clears the lifetime counter, so the impact follows on the next frame.

:::caution[A value of zero suppresses the bounce effects on a landing]
Debris that keeps none of its speed has nothing left to carry it back off the ground, so its arrival there is reported as settling rather than as a strike. [`BounceAnim`](/keys/bounceanim/#scope-voxelanimtype), [`BounceSound`](/keys/bouncesound/#scope-voxelanimtype) and the bounce damage all hang off a strike, so a piece that simply falls and stops runs none of them.
:::

A figure above `1` hands back more speed than the piece arrived with, and the piece climbs higher after each contact until [`Duration`](/keys/duration/) runs out.
