---
key: Reverse
summary: Plays the animation from its loop end back down toward its start frame.
see_also: ["LoopEnd", "Start", "LoopCount", "PingPong"]
when_omitted:
  kind: value
  value: "no"
---

A reversed animation plays its frames backward. It opens on frame [`LoopEnd`](/keys/loopend/) and steps down one frame at a time. The pass ends when it steps to frame `0`, so frame `0` is never shown. Every further pass begins on `LoopEnd` again, so [`LoopStart`](/keys/loopstart/) has no effect.

These frame numbers assume [`Start`](/keys/start/) is `0`, the usual case. A non-zero `Start` shifts both ends of a reversed pass, as that page explains.

:::caution[Set LoopEnd to the last frame]
When neither `LoopEnd` nor [`End`](/keys/end/) is set, `LoopEnd` is the shape's frame count, one past its last frame. A reversed animation then opens on a frame that draws nothing. Set `LoopEnd=` to the frame count minus one, the number of the last frame, as in the example below.
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

The direction is set only when the animation is created. An animation that changes into this type through [`Next=`](/keys/next/) keeps the direction it already had, so `Reverse=yes` does not turn it around.

:::caution[Do not give a reversed animation a Next type]
After a reversed animation changes type through `Next=`, it keeps stepping backward from the new type's start. If the new type does not set `Reverse=yes`, the animation keeps stepping backward past frame `0` of the shape, then draws nothing and never finishes. When the new type's `Start` is above `0`, it first opens on frame twice that `Start` and plays every frame below it. If the new type sets `Reverse=yes`, its first pass ends after one frame.
:::
