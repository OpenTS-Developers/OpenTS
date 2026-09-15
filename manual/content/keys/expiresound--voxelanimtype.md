---
key: ExpireSound
scope: voxelanimtype
label: Voxel debris impact sound
see_also: ["ExpireAnim", "BounceSound", "StartSound"]
when_omitted:
  kind: value
  value: none
---

The sound plays where the debris ends its life, unless that end comes low over water, where only the splash is heard. It is tested on its own rather than inside the [`ExpireAnim`](/keys/expireanim/#scope-voxelanimtype) branch that deals the blast damage, so a piece with no impact animation still plays it.

A value naming no registered sound leaves whatever was set before in place.
