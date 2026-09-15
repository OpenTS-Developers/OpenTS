---
key: TrailerSeperation
summary: The number of frames between one trail animation and the next.
see_also: ["TrailerAnim"]
when_omitted:
  kind: value
  value: "0"
---

The animation sheds a [`TrailerAnim`](/keys/traileranim/#scope-animtype) on every frame of the game clock whose number is a multiple of this figure. `1` leaves one every frame and `4` leaves one every fourth. The clock is the game's and not the animation's own, so two animations of the same type created a frame apart still drop their trails together.

The setting is read only where a trailer animation is named. A negative figure behaves as its magnitude, since the test asks only whether the frame divides evenly.

:::danger[A trail without a separation stops the game]
The test divides by this figure and nothing guards the zero it holds when the section leaves it out. An animation that names a trailer animation without a separation faults on its first logic frame, so the game stops the moment one is created. Every animation in the shipped `art.ini` that names a trailer gives a separation with it.
:::
