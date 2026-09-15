---
key: AnimationRate
summary: Game frames each frame of an animated terrain object's artwork is held for.
see_also: [IsAnimated, AnimationProbability]
when_omitted:
  kind: value
  value: "0"
  note: A pace of zero, which never advances the artwork.
---

The figure is only ever applied by [`IsAnimated=yes`](/keys/isanimated/), and only at the moment a stopped animation is started again. It is a countdown in game frames: the artwork advances one frame each time that many pass. `1` advances every frame (fifteen frames a second) and `15` advances once a second.

```ini title="rules.ini"
[MYTREE]                 ; example blossom tree
IsAnimated=yes
AnimationRate=3          ; one frame every three game frames, five a second
AnimationProbability=.02
SpawnsTiberium=yes
```

:::caution[A pace of zero leaves the object stuck on its first frame]
Zero is not a pace but a stopped animation, so starting the animation with it stops the animation again in the same step. The object is returned to its first frame, and because it counts as stopped it is eligible to be started again on the very next frame. That start returns it to the first frame once more, for as long as the scenario runs.
:::
