---
key: LightIntensity
summary: The brightness of the glow a structure casts over the ground around it.
see_also: [LightVisibility, LightRedTint, LightGreenTint, LightBlueTint, HasSpotlight, "system:power"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[GALITE] ; the stock light post
LightVisibility=5000
LightIntensity=0.2
LightRedTint=0.05
LightGreenTint=0.05
LightBlueTint=0.01
```

The figure is a fraction of full brightness on the same scale the map's own ambient light uses. A structure with `LightIntensity=1` adds as much light at its own cell as a fully lit map already has, and the contribution falls away in a straight line to nothing at [`LightVisibility`](/keys/lightvisibility/). The contribution itself does not scale with the map's ambient light; only the brightness it lands on does. A negative figure darkens instead, which is how the stock negative light post takes light out of the ground around it.

A figure of exactly zero means the structure has no light source at all, and the radius and the three tints are then never read. Otherwise the glow is created with the structure, switched off while it is unpowered, and destroyed with it; [Fields, fences and lights](/systems/power/#fields-fences-and-lights) covers the power condition. The glow is unrelated to the swept beam that [`HasSpotlight`](/keys/hasspotlight/) covers.

A file read after `rules.ini` (an expansion rules file, or a scenario's own rules) that names the same section without this key cuts the stored figure back to a whole number before storing it again. A `0.2` light set in the rules therefore goes out entirely the moment a map touches that section for any other reason, and a `1.5` light drops to `1`. The three tints round off the same way.

:::caution[Set the tints whenever you set the intensity]
The three tint keys start at `1000`, against stock figures that all sit between `-1.5` and `2`, and a structure that sets this key without setting all three of them keeps that starting figure. The tints of a cell are normalized against each other before the ground is drawn, so such a light adds no color at all. The scale divided out of them is multiplied into the cell's brightness, and being that large it runs past the range that brightness is computed in. The cells inside the radius then sit at full brightness or at none rather than in an even pool.
:::
