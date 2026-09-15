---
key: End
summary: The number of stages the animation runs through before it ends or chains.
see_also: ["Start", "LoopEnd", "LoopCount", "Next", "Rate"]
when_omitted:
  kind: computed
  note: The frame count of the shape the type first gets hold of, or zero if it never gets one.
---

The figure is a count of stages, not the number of a last frame. A stage is one position of the animation's frame counter, which starts at zero. The frame drawn at a stage is [`Start`](/keys/start/) plus the stage number, so the frames displayed are `Start` through `Start` plus this figure minus one. The stage that reaches the figure itself is never displayed. The animation is removed, or turns into its [`Next=`](/keys/next/) animation, in the same logic pass that arrives at it.

Only the last pass ends here. While an animation still has passes left to run, [`LoopEnd`](/keys/loopend/) ends each of them instead, and this figure decides only where the final one stops. Without [`LoopCount=`](/keys/loopcount/), the animation plays its stages once.

The count is not bounded by the artwork. An animation that takes a sub-sequence with `Start=` but leaves this figure to the shape's own frame count runs from its start frame for the whole length of the shape. The run continues past the last frame the shape holds; those frames draw nothing, so the animation finishes with a stretch of empty stages before it disappears.

The figure is also the length the by-stage fade measures itself against, so shortening it moves the points at which a [`Translucent=yes`](/keys/translucent/#scope-animtype) animation steps to its next level.

## Where the stored count comes from

An animation type measures itself from its shape the first time it gets hold of one, and this read passes that measurement through, which is why most animations state nothing here. The measurement is taken once. A type whose ID-named shape was found keeps that frame count even if a later [`Image=`](/keys/image/#scope-animtype) points at a different shape. A type that never finds a shape is left with a count of zero and plays no stages at all.

`-1` marks the count unset, and the frame count is taken from the artwork again the next time an animation of the type is created, then stored on the type for good.

:::danger[Marking the count unset on an animation with no artwork faults]
The refetch reads the frame count straight from the animation's shape without first checking that there is one. An `End=-1` animation whose shape cannot be found (a missing file, or a name the theater rewrote into one) faults the moment anything creates an animation of that type.
:::
