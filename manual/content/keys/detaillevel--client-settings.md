---
key: DetailLevel
scope: client-settings
label: Chosen detail level
see_also: [TranslucencyDetailLevel, GameSpeed]
when_omitted:
  kind: value
  value: "2"
---

The figure runs from `0` for low through `2` for high. The read holds it to that range, so a figure outside it is pulled back to the nearer end rather than reaching the renderer. The in-game game controls dialog offers the three positions and writes the choice back to `sun.ini`.

Each step turns on more of the drawing work rather than scaling one dial. Smoke and spark particles, and every laser beam, are skipped entirely at `0`. Cell lighting is quantized at every setting the dialog can reach, coarsely at `0` and least so at `2`. Only `2` draws the sonic wave beam and the ion blast flare, applies a particle's translucency, and lets a particle system cast its spotlights.

An animation is drawn only where the setting has reached the animation's own threshold; [`DetailLevel`](/keys/detaillevel/#scope-animtype) on an AnimType covers that test. The picture remembered under the fog of war goes through the same test before it is drawn.

:::note[The lighting quantization has an unreachable fourth case]
The quantization is chosen by a four-way selection whose last case skips quantization altogether, for a figure of `3` or more. The read clamps to `2` first, so nothing selects it.
:::
