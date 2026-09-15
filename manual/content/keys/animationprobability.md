---
key: AnimationProbability
summary: Chance each game frame that a stopped terrain animation starts over.
see_also: [IsAnimated, AnimationRate]
when_omitted:
  kind: value
  value: "0"
  note: No roll ever succeeds, so an animation that has stopped never restarts.
---

The figure is a fraction between `0` and `1`, and it is only read on an [`IsAnimated=yes`](/keys/isanimated/) object whose animation is currently stopped. Each game frame such an object draws a random fraction and starts its animation when the draw falls below this figure, so `.02` gives one chance in fifty per frame. On average that is a start every fifty frames, a little over three seconds at fifteen frames a second. A percentage may be written instead, and `2%` means the same as `.02`.

```ini title="rules.ini"
[MYTREE]                 ; example blossom tree
IsAnimated=yes
AnimationRate=3
AnimationProbability=.02 ; one restart every 50 frames on average
SpawnsTiberium=yes
```

The random draw has six digits of resolution and never quite reaches `1`, so `1` and anything above it restarts the animation on every frame that finds it stopped. Anything at or below `0` never restarts it, which leaves an animation that has run its course stopped for good.
