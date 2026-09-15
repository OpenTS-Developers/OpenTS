---
key: BlendedFog
summary: Parsed flag whose branches are unreachable.
no_effect: true
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "yes"
---

The flag is read in exactly two places, both inside the routine that draws a terrain tile. Both are behind a parameter saying the tile is being drawn as fog. Neither of the two calls into that routine passes it, so the checkerboard alternative the flag selects is never reached, and `BlendedFog=no` draws nothing differently.

Fog is drawn from its own artwork over the alpha buffer instead, which is why the flag makes no visible difference.
