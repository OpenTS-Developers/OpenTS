---
key: TrailerAnim
scope: animtype
label: Animation trail
see_also: ["TrailerSeperation", "Bouncer", "IsMeteor"]
when_omitted:
  kind: value
  value: none
---

For as long as the animation is alive, it creates an animation of the named type at its own position. The creations happen on every frame of the game clock whose number is a multiple of [`TrailerSeperation`](/keys/trailerseperation/). The count is the clock's rather than the animation's own, so every animation of the type sheds its trail on the same frames. A trailer begins one frame after the animation that sheds it is created.

Nothing restricts this to thrown animations. An explosion or a smoke column that names a trailer sheds one just as a meteor does.

A value naming no registered animation is not refused. A type of that name is created on the spot, holding no artwork and every setting at its built-in value.

:::danger[A trail without a separation stops the game]
`TrailerSeperation` is `0` unless the section sets it, and the frame test divides by it. Naming a trailer animation without also giving a separation faults on the animation's first logic frame, so the game stops the moment an animation of the type is created.
:::
