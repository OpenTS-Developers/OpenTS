---
key: RandomRate
summary: The pair of playback rates a new animation picks its frame delay from.
see_also: ["Rate", "Normalized", "RandomLoopDelay"]
when_omitted:
  kind: value
  value: "0,0"
  note: Both bounds come out at zero stored waits, so the type has no random span and the flat rate is used.
---

Both numbers are animation frames per minute of game time. Each is stored as `900` divided by it in whole numbers, the same conversion [`Rate=`](/keys/rate/#scope-animtype) uses. A wait is picked from the whole numbers between the two stored bounds when an animation of the type is created, and picked again when an animation chains into the type. While both bounds are zero the flat rate is used instead.

A rate of `0` or below stores no wait for that bound.

:::caution[The faster rate has to be written first]
The two numbers are stored as waits, so the higher rate becomes the lower bound. If the first number produces a longer wait than the second, the lower bound is raised to meet the upper one, and every animation of the type is given that single wait with no randomness at all. `RandomRate=600,220` picks a wait of between one and four frames; `RandomRate=220,600` describes the same span the other way round and gives a wait of exactly one frame every time.
:::

A value with only one number is discarded whole. Both halves have to be written.

[`Normalized=yes`](/keys/normalized/#scope-animtype) rescales whichever wait comes out of the pick.
