---
key: FirestormAirAnim
summary: The animation a raised firestorm wall creates when what it catches is more than 100 leptons above the ground.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

It is created at the victim's own position rather than the wall's, centered and drawn in front of it. It runs its animation type's own [`LoopCount`](/keys/loopcount/), which is a single repetition unless the type sets one. The height test, more than 100 [leptons](/glossary/#lepton) above the ground, is the only thing that chooses between this animation and [`FirestormGroundAnim`](/keys/firestormgroundanim/); the flags, depth adjustment and loop count are identical. Both are created by every path that catches something: the sweep of a section's own cell, a flying object or jumpjet crossing it, and a projectile [consumed by the wall](/systems/laser-fences/#projectiles). The projectile path produces the animation without applying any damage.

:::danger[An unset value faults the moment the animation is due]
The animation is built from this setting without checking that it resolved to anything. With no value set the pointer is empty, and the first time a raised section catches something above that height, the engine builds an animation from it and reads through that empty pointer straight away.
:::
