---
key: Start
summary: The frame of the shape the animation's first stage displays.
see_also: ["End", "LoopStart", "LoopEnd", "Reverse", "Next"]
when_omitted:
  kind: value
  value: "0"
---

Every stage the animation reaches is added to this figure to pick the frame drawn. The animation therefore occupies the run of frames from here upward, which is what lets several animations share one shape file, each taking a different stretch of it.

The frame at this index is the one displayed while the animation is still on its first stage. Nothing checks the figure against the artwork: one past the last frame in the shape leaves the animation drawing nothing for its whole life.

## What is counted from here and what is not

[`End`](/keys/end/) is a count of stages measured from this figure, so the frames a single pass covers are this figure through this figure plus `End` minus one. [`LoopStart`](/keys/loopstart/) and [`LoopEnd`](/keys/loopend/) are not counts; they are frame numbers in the shape, and the engine subtracts this figure from each of them to turn it back into a stage. The three therefore fit together only when the count and the loop range describe the same stretch.

```ini title="art.ini"
[MYPLUG_D] ; the damaged form of an animation sharing MYPLUG's shape
Image=MYPLUG
Start=10      ; the damaged sequence begins at frame 10
LoopStart=10  ; a frame number, not a count
LoopEnd=20    ; ends each pass on frame 20, which is not displayed
End=10        ; ten stages, so the final pass also covers frames 10 through 19
LoopCount=3
```

Leaving `End` out of that section would not shorten the animation to match. The stage count is taken from the whole shape, so the final pass would run from frame 10 for the shape's full length, off the end of the artwork.

:::caution[The loop block mixes stages and frame numbers]
The reverse and ping-pong end tests, and the stage a chained animation is put on, treat this frame number as though it were a stage. On an animation whose figure is zero, the ordinary case, the two agree and nothing is out of place. On one that sets it, a [`Reverse=yes`](/keys/reverse/) animation stops once its stage reaches this figure. The drawn frame is this figure plus the stage, so the pass ends at twice this figure (frame 20 in the example above) instead of at this figure itself. An animation reached through [`Next=`](/keys/next/) opens on twice this figure as well.
:::

The stage at which an animation lays down its scorch mark or crater is compared against the frame number rather than the stage, so that moment stays pinned to the artwork whatever this is set to.
