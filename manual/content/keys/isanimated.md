---
key: IsAnimated
summary: Terrain object that plays through its artwork instead of holding a single frame.
see_also: [AnimationRate, AnimationProbability, SpawnsTiberium]
when_omitted:
  kind: value
  value: "no"
---

The flag turns on three separate behaviors at once.

Once per frame, an object of the type whose animation is stopped rolls against [`AnimationProbability`](/keys/animationprobability/) to start it again. A successful roll returns the object to its first frame and sets the pace from [`AnimationRate`](/keys/animationrate/). The frame drawn is then the animation's own, which displaces both the damaged frame an ordinary terrain object shows below two health and the crumbling frames a dying one shows. And the object stops writing its silhouette into the depth buffer for as long as the flag is set, so that a shape changing from frame to frame cannot stamp a stale outline there.

```ini title="rules.ini"
[MYTREE]              ; example blossom tree
IsAnimated=yes
AnimationRate=3
AnimationProbability=.02
SpawnsTiberium=yes
```

:::caution[Only a Tiberium-spawning type stops at the end of its artwork]
An animated [`SpawnsTiberium=yes`](/keys/spawnstiberium/) object returns to its first frame and halts when it reaches the middle of its shape file, the point where the shadow frames begin, and seeds the ground as it does so. On any other animated terrain type nothing stops the count. The frame number walks on into the shadow frames, so the object shows its own shadow artwork as its body. Past the last frame the file holds, every draw asks for a frame that is not there and puts nothing on the screen.
:::
