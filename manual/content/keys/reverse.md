---
key: Reverse
summary: Plays the animation from its loop end back down toward its start frame.
see_also: ["LoopEnd", "Start", "LoopCount", "PingPong"]
when_omitted:
  kind: value
  value: "no"
---

A stage is the animation's frame counter, and the frame drawn is the type's start frame plus it. A reversed animation is created on the stage held by [`LoopEnd`](/keys/loopend/) and steps down one stage at a time, finishing once the stage arrives at [`Start`](/keys/start/). Every further pass begins on the loop end again, and [`LoopStart`](/keys/loopstart/) is not read at all.

The flag is applied only when the animation is created. One that reaches this type through [`Next=`](/keys/next/) is put on the type's start figure as a stage but keeps stepping the way it already was. The flag on the type it arrived at is never read.

:::caution[A reversed animation usually opens on a blank frame]
The loop end is put on the animation as a stage rather than as a frame number, so the first frame displayed is the start frame added to the loop end. On the ordinary animation, whose start frame is zero, that is the loop end itself. With the loop end left at the shape's own frame count, that frame is one past the last frame the shape holds and draws nothing. Setting `LoopEnd=` to the number of the last frame rather than to the frame count removes the blank opening frame.
:::

```ini title="art.ini"
[MYREFN_AR] ; MYREFN_A played backwards, over a six frame shape
Image=MYREFN_A
LoopStart=0
LoopEnd=5   ; the last frame, so the animation opens on artwork
LoopCount=1
Reverse=yes
Rate=200
Surface=yes
```
