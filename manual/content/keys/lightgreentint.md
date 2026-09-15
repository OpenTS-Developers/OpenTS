---
key: LightGreenTint
summary: The green the glow of a structure adds to the cells it reaches.
see_also: [LightRedTint, LightBlueTint, LightIntensity, LightVisibility]
when_omitted:
  kind: value
  value: "1000"
---

```ini title="rules.ini"
[GRENLAMP] ; the stock green light post
LightVisibility=4000
LightIntensity=0.01
LightRedTint=0.01
LightGreenTint=1.5
LightBlueTint=0.01
```

The figure is a fraction of full brightness on the same scale the map's own color grading uses. `1` adds as much green at the structure's center as a fully lit map already has, and the addition falls away to nothing at [`LightVisibility`](/keys/lightvisibility/), exactly as [`LightIntensity`](/keys/lightintensity/) does. A negative figure takes green out of the cells instead.

What reaches the screen is the balance between the three tints rather than their size, which [`LightRedTint`](/keys/lightredtint/) covers.

A file read after `rules.ini` that names the same section without this key cuts the stored figure back to a whole number before storing it again. A `1.5` green drops to `1`; a `0.01` green drops out of the light altogether.
