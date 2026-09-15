---
key: LightRedTint
summary: The red the glow of a structure adds to the cells it reaches.
see_also: [LightGreenTint, LightBlueTint, LightIntensity, LightVisibility]
when_omitted:
  kind: value
  value: "1000"
---

```ini title="rules.ini"
[REDLAMP] ; the stock red light post
LightVisibility=4000
LightIntensity=0.01
LightRedTint=1.5
LightGreenTint=0.01
LightBlueTint=0.01
```

The figure is a fraction of full brightness on the same scale the map's own color grading uses. `1` adds as much red at the structure's center as a fully lit map already has, and the addition falls away to nothing at [`LightVisibility`](/keys/lightvisibility/), exactly as [`LightIntensity`](/keys/lightintensity/) does. A negative figure takes red out of the cells instead, which is what the stock negative red light is built from.

What reaches the screen is the balance between the three tints rather than their size. Before the ground is drawn, a cell's three channels are divided through by the strongest of them, and the cell's brightness is multiplied by that same factor, so the division leaves the cell as bright as it was. Raising all three together therefore brightens the ground rather than coloring it, and one channel well above the others is what gives a light its color.

A file read after `rules.ini` that names the same section without this key cuts the stored figure back to a whole number before storing it again. A `1.5` red drops to `1`; a `0.05` red drops out of the light altogether.
