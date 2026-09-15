---
key: BounceSound
scope: voxelanimtype
label: Voxel debris bounce sound
see_also: ["BounceAnim", "StartSound", "ExpireSound"]
when_omitted:
  kind: value
  value: none
---

The sound plays at the debris' position on every frame the bounce physics report a strike, under exactly the conditions [`BounceAnim`](/keys/bounceanim/#scope-voxelanimtype) describes. It is not played in a water cell, and not on the contact that settles the piece.

A value naming no registered sound leaves whatever was set before in place rather than clearing it, so a misspelled name is silently ignored.
