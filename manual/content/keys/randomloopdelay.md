---
key: RandomLoopDelay
summary: The bounds, in game frames, of the pause inserted between one pass of a looping animation and the next.
see_also: ["LoopCount", "RandomRate", "Rate", "Report"]
when_omitted:
  kind: value
  value: "0,0"
---

A pause is drawn from between the two bounds each time a pass ends and another begins, so the pair needs [`LoopCount`](/keys/loopcount/) above one to do anything. While both bounds are zero the animation loops with no pause at all. Fifteen frames make a second of game time, so `RandomLoopDelay=10,300` is a pause of between two thirds of a second and twenty seconds.

The animation does not advance while it is paused, and nothing marks it for redraw, so the frame it stopped on stays in place. When the pause runs out, the animation's opening side effects run again. Its [`Report=`](/keys/report/#scope-animtype) sound plays again, and a [`TiberiumChainReaction=yes`](/keys/tiberiumchainreaction/) animation sets off the tiberium beneath it again. An animation whose largest frame is its first performs its scorch or crater step again, but the new mark lands only where the ground has no smudge yet, so a stationary animation does not deepen the first one. A looping animation with a sound and a pause therefore repeats that sound once per pass, which is what makes the pause worth having on an idle effect and unusable on a noisy one.

A value with only one number is discarded whole, and both bounds keep whatever the type already holds. Both halves have to be written.
