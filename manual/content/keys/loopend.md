---
key: LoopEnd
summary: The frame that ends every pass of a looping animation except the last.
see_also: ["LoopStart", "LoopCount", "End", "Reverse", "TurretAnim"]
when_omitted:
  kind: computed
  note: The End stage count, taken as a frame number.
---

The value is a frame number in the shape. [`End`](/keys/end/), by contrast, counts stages from [`Start`](/keys/start/). When a pass reaches this frame, the animation jumps back to [`LoopStart`](/keys/loopstart/) before the frame is drawn, so the `LoopEnd` frame itself never appears.

It ends every pass except the last. The last of the [`LoopCount`](/keys/loopcount/) passes runs to the end of the `End` stage count instead. If the two settings disagree, the last pass plays a different stretch of the shape than the passes before it.

An omitted `LoopEnd`, `LoopEnd=0` and `LoopEnd=-1` all take the `End` stage count: the `End` that is set, or the shape's frame count when `End` is omitted, `0` or `-1`. Setting `End=` alone therefore ends the middle passes at the same number. That count is treated as a frame number, so with a nonzero `Start` the middle passes stop `Start` frames short of where the last pass ends.

A [`Reverse=yes`](/keys/reverse/) animation starts every pass on frame `Start` plus `LoopEnd` and steps backward. With a nonzero `Start` and more than one pass, each reversed pass ends at its first frame advance, as [`Start`](/keys/start/#what-is-counted-from-here-and-what-is-not) describes.

A structure with a charging weapon ([`Charges=yes`](/keys/charges/)) uses this value from the animation in its [`TurretAnim`](/keys/turretanim/) slot. The weapon is charged once the charge sequence reaches that frame.
