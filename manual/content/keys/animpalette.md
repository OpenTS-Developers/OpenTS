---
key: AnimPalette
summary: Draws the projectile's body with the animation palette instead of the normal one.
see_also: [Image, Voxel]
when_omitted:
  kind: value
  value: "no"
---

Only the body is remapped. The shadow thrown under a projectile that is above the ground is drawn from the normal palette either way, so a remapped projectile keeps an ordinary shadow. A [`Voxel=yes`](/keys/voxel/) projectile is drawn through the voxel path, which takes its colors from [`Color`](/keys/color/#scope-bullettype) and never reads this setting.

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it the lookup runs against an empty section name, so nothing here is read, not even from an art section named after the projectile, which is where its `Voxel` setting is still found.
