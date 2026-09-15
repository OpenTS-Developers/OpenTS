---
key: LoopEnd
summary: The frame that ends every pass of a looping animation except the last.
see_also: ["LoopStart", "LoopCount", "End", "Reverse", "TurretAnim"]
when_omitted:
  kind: computed
  note: The frame count of the shape the type first gets hold of, or zero if it never gets one.
---

The value is a frame number in the shape rather than a count of stages, which is how it differs from [`End`](/keys/end/). The frame itself is not displayed: the pass ends and the animation jumps back to [`LoopStart`](/keys/loopstart/) in the same logic pass that reaches it.

It bounds the middle passes only. The final pass, the one during which the last of the [`LoopCount`](/keys/loopcount/) is spent, runs to the stage count in `End` instead. An animation whose two figures disagree plays a different length of artwork on its way out than it did on the way round.

A [`Reverse=yes`](/keys/reverse/) animation begins here rather than ending here, and returns here at the start of every pass.

Shortening an animation with `End=` does not move this figure. The stage count and the loop end are both taken from the artwork before either key is read, and this key is read after `End=`. An animation that states only the shorter stage count therefore still loops over the full length of its shape. Both have to be stated together to shorten a looping animation.

`-1` marks the figure unset, and it is taken from the animation's stage count again the next time an animation of the type is created.

A structure whose weapon charges up measures the charge against the frame number belonging to the animation in its [`TurretAnim`](/keys/turretanim/) slot. The turret is charged once the charge sequence reaches that frame, or once it reaches frame 12 if the structure runs no turret animation at all.
