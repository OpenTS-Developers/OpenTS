---
key: ExpireAnim
scope: voxelanimtype
label: Voxel debris impact effect
see_also: ["Damage", "Warhead", "ExpireSound", "Duration"]
when_omitted:
  kind: value
  value: none
---

An animation of the named type is created where the debris ends its life, provided that end does not come low over water. Solid ground, a bridge deck, and anything more than about a cell and a half above the ground all take this path. A piece that ends low over water makes a wake and a small splash instead (a meteor plays the last animation in the rules' splash list), and this setting is not read there.

:::caution[The impact blast sits inside the same branch]
[`Damage`](/keys/damage/#scope-voxelanimtype) and the flash of light that goes with a bright warhead are applied in the same block as the animation. A piece with no animation named here therefore deals no blast damage whatever its damage figure and warhead say. The damage dealt on a bounce is a separate path and still applies.
:::
