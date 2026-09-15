---
key: AnimIdle
summary: The first frame, frame count and step delay of a structure's resting animation.
see_also: ["AnimActive", "AnimAux1", "AnimAux2", "ConditionYellow"]
when_omitted:
  kind: value
  value: "0,1,0"
---

The value is three whole numbers: the first frame of the structure's own artwork to draw, how many frames the sequence runs for, and how many game frames each one is held. A delay of `0` leaves the sequence standing on its first frame, and a sequence that runs past its last frame starts again at the first.

```ini title="art.ini"
[MYSILO] ; example missile silo, drawn from its own Image ID
AnimIdle=0,2,8   ; two frames, eight game frames each
AnimActive=2,5,4 ; the door opening
```

This is the sequence a structure runs whenever nothing else has taken it over. Each time the sequence runs past its last frame, its delay is adjusted for the selected game speed. The same adjustment applies each time a structure that has already been running something else drops back into it. The working and auxiliary sequences are used as written. A structure placed on the map has no earlier state to drop back from, so its first idle cycle runs at the delay written in the art entry unless the type sets [`Normalized=yes`](/keys/normalized/). It corrects itself on the first wrap.

The four sequences together fix where the damaged artwork begins. A damaged structure running this sequence draws one frame further along. A damaged structure running any of the other three is offset by the largest end any of the four reaches, so the damaged block has to start after all of them.

:::caution[A value with fewer than three numbers leaves the rest as they were]
The three numbers are taken in order and only as many as the value supplies are stored. `AnimIdle=5` sets the first frame and leaves the frame count and delay at whatever they already held, which for a type that sets nothing else is one frame at no delay.
:::
