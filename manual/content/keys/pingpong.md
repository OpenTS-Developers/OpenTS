---
key: PingPong
summary: Reverses the animation's direction each time it reaches either end of its frames.
see_also: ["LoopCount", "LoopEnd", "End", "Reverse", "Start"]
when_omitted:
  kind: value
  value: "no"
---

On reaching either end of its range the animation flips its step and walks back the way it came, instead of snapping to the start or finishing. A stage is the animation's frame counter, and the frame drawn is the type's start frame plus it. Which ends it turns at depends on [`LoopCount`](/keys/loopcount/). An animation left on a single pass turns at the stage count in [`End`](/keys/end/) and at its first stage. One given more than one pass turns at [`LoopEnd`](/keys/loopend/) and at the stage matching [`Start`](/keys/start/).

The stage that triggered a turn is held for one frame before the animation steps back, so the frame at the top of the range is displayed. With the range left at the shape's own frame count, that held frame is one past the last frame in the shape and draws nothing. The animation blinks each time it turns at the top of the range. Setting the range one frame shorter removes it.

:::caution[A ping-pong animation does not finish on its own]
The turn is tested first, and at the top of the range its condition is the very one the end of a pass uses. The end-of-pass bookkeeping therefore never runs: the animation spends no passes, never chains through [`Next=`](/keys/next/), and never removes itself. It plays until something else takes it away: the structure whose slot it fills stops it, the object it is pinned to is destroyed, or the scenario ends. An animation created for its own sake with this flag set stays in the game for the rest of the mission. The one way out is [`Reverse=yes`](/keys/reverse/) on an animation left on a single pass with a non-zero [`Start`](/keys/start/). The turn's low end is stage zero, while the reverse finish test fires when the stage steps down to the Start figure, so the animation finishes before it can turn.
:::
