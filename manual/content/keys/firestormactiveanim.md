---
key: FirestormActiveAnim
summary: The animation a raised firestorm wall section runs while it is not part of a straight run.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: none
---

The animation is created half a cell up and to the left of the section, centered and drawn in front of it. It inherits the section's fogged state. A section takes one when its house has [raised the wall](/systems/laser-fences/#raising-and-lowering-the-wall) and its connection shape is neither of the two straight runs (north-to-south and east-to-west). Corners, tees, crossings and isolated sections are lit; a plain stretch of wall is not. A refresh that finds a section already running one removes it rather than leaving it in place, so a section refreshed twice in a row ends up unlit. The pass runs when the wall goes up or down, and whenever a section is placed or taken off the map.

:::danger[An unset value faults the moment the animation is due]
The animation is built from this setting without checking that it resolved to anything. With no value set the pointer is empty, and the first time a raised section takes one, the engine builds an animation from it and reads through that empty pointer straight away.
:::

