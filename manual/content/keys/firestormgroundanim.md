---
key: FirestormGroundAnim
summary: The animation a raised firestorm wall creates when what it catches is at or below 100 leptons above the ground.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

It is created at the wall section's own position rather than the victim's, centered and drawn in front of it. It runs its animation type's own [`LoopCount`](/keys/loopcount/), which is a single repetition unless the type sets one. [`FirestormAirAnim`](/keys/firestormairanim/) is the alternative taken above 100 [leptons](/glossary/#lepton) and covers which paths create either.

The [approach sweep](/systems/laser-fences/#what-a-raised-section-destroys), the check that destroys anything trying to move into a raised section's cell, is the one lethal path that produces no animation at all: its victim dies short of the section, without either of these.

:::danger[An unset value faults the moment the animation is due]
The animation is built from this setting without checking that it resolved to anything. With no value set the pointer is empty, and the first time a raised section catches something at or below that height, the engine builds an animation from it and reads through that empty pointer straight away.
:::
