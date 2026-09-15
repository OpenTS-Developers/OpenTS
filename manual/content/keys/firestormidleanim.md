---
key: FirestormIdleAnim
summary: The animation a raised firestorm wall section flickers over itself at random.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

Every eighth frame, a section whose house has [raised the wall](/systems/laser-fences/#raising-and-lowering-the-wall) and whose connection shape is neither of the two straight runs rolls a one-in-sixteen chance to start one. It is drawn half-transparent, nearly three cells up and to the left of the section. Only one is runs at a time: the roll is skipped while the previous one is still playing.

:::danger[An unset value faults the moment the animation is due]
The animation is built from this setting without checking that it resolved to anything. With no value set the pointer is empty, and the first time the roll comes up on a raised section, the engine builds an animation from it and reads through that empty pointer straight away.
:::
