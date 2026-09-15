---
key: LoopStart
summary: The frame a looping animation returns to at the start of each new pass.
see_also: ["LoopEnd", "LoopCount", "Start", "Reverse"]
when_omitted:
  kind: value
  value: "0"
---

The value is a frame number in the shape, not a count from [`Start`](/keys/start/). It is read only when a pass has just ended and the animation still has passes left, so it needs [`LoopCount`](/keys/loopcount/) above one to do anything; the opening pass always begins on the animation's start frame whatever this says.

```ini title="art.ini"
[MYANIM] ; an animation of a 60-frame shape
Start=10
LoopStart=20
LoopCount=3
```

A [`Reverse=yes`](/keys/reverse/) animation ignores it entirely: a reversed pass restarts on [`LoopEnd`](/keys/loopend/) and walks back down.

A frame number below the animation's start frame is turned back into itself: the engine subtracts the start frame to get a stage and adds it again to draw, so the pass restarts on exactly that frame. A figure past the end of the shape is not corrected. One far enough above the start frame restarts the animation past the end of its own stage count. The pass then ends again on the next advance, and the animation burns through its remaining passes at one per frame delay.
