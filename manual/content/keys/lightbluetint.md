---
key: LightBlueTint
summary: The blue the glow of a structure adds to the cells it reaches.
see_also: [LightRedTint, LightGreenTint, LightIntensity, LightVisibility]
when_omitted:
  kind: value
  value: "1000"
---

```ini title="rules.ini"
[BLUELAMP] ; the stock blue light post
LightVisibility=4000
LightIntensity=0.01
LightRedTint=0.01
LightGreenTint=0.01
LightBlueTint=0.7
```

The figure is a fraction of full brightness on the same scale the map's own color grading uses. `1` adds as much blue at the structure's center as a fully lit map already has, and the addition falls away to nothing at [`LightVisibility`](/keys/lightvisibility/), exactly as [`LightIntensity`](/keys/lightintensity/) does. A negative figure takes blue out of the cells instead.

What reaches the screen is the balance between the three tints rather than their size, which [`LightRedTint`](/keys/lightredtint/) covers.

A file read after `rules.ini` that names the same section without this key cuts the stored figure back to a whole number before storing it again, so a `0.7` blue drops out of the light altogether.
