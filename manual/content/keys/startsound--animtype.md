---
key: StartSound
scope: animtype
label: Animation creation sound
no_effect: true
see_also: ["Report", "BounceSound", "ExpireSound"]
when_omitted:
  kind: value
  value: none
---

The name promises the sound an animation makes as it is created. The assignment is accepted and the sound is stored on the type. Nothing on an animation ever looks at it again: not the creation path, not the bounce physics, and not the multiplayer synchronization checksum, which passes over the field.

The sound an animation actually plays for itself is the one [`Report`](/keys/report/#scope-animtype) names. A voxel animation of the same name does play its own creation sound.
