---
key: Shadow
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: [Voxel, High]
when_omitted:
  kind: value
  value: "yes"
---

The name promises control over the shadow drawn beneath a projectile in flight. That shadow is drawn under every shape-drawn projectile that is above the ground and under no [`Voxel=yes`](/keys/voxel/) projectile, and neither case reads this flag: `Shadow=no` removes nothing.

No gameplay or rendering path reads the stored flag.
