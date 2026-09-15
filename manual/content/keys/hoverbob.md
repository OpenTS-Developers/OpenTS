---
key: HoverBob
summary: Period in minutes of the vertical bob a hovering unit performs.
see_also: [HoverHeight, HoverDampen]
when_omitted:
  kind: value
  value: "30"
---

The bob is a sine wave laid over whatever height the cushion has settled on. Only its period is configurable: the amplitude is a fixed two leptons up or down, against the 256 leptons that make a cell. The figure is converted at 900 frames to the minute, so the stock `.04` gives a cycle of 36 frames, a little over two seconds.

Hover units are split between exactly this period and 1.1 times it, and each runtime instance enters the cycle at its own point, so a group of them does not rise and fall in unison. The levitation locomotor bobs on the same figure.

:::danger[A period under one frame crashes the game]
The period is truncated to whole frames and then used as a divisor. A value below `1/900` of a minute truncates to zero, and the engine takes a remainder by zero as it computes the bob.
:::
