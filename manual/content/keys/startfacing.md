---
key: StartFacing
summary: The facing a structure snaps to when its construction animation ends, in eighths of a turn.
see_also: [StartPitch, IsJuggernaut, Artillary, LaserFence]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[DJUGG] ; the stock deployed Juggernaut
IsJuggernaut=yes
StartFacing=4 ; south
```

The figure counts eighths of a turn: `0` north, `2` east, `4` south, `6` west. The engine multiplies the figure onto its 256-step dial, so each step is 45 degrees and there is no way to write a facing between two of them. A figure of `8` or more wraps around.

The facing is applied the moment a structure's construction animation finishes. A [`LaserFence=yes`](/keys/laserfence/) structure is skipped and keeps whatever facing it was placed with. A structure pre-placed on the map never plays a construction animation, so this facing does not reach it.

An [`IsJuggernaut=yes`](/keys/isjuggernaut/) structure will not begin to fold away until its body is back at this facing and its barrel is back at [`StartPitch`](/keys/startpitch/). Deconstruction turns both toward their start values and retries every frame until they arrive, before the reverse build-up runs. An [`Artillary=yes`](/keys/artillary/) structure takes the same paired test later on, at the point its [`UndeploysInto`](/keys/undeploysinto/) vehicle would be created.
