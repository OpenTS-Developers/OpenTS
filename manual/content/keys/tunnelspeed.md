---
key: TunnelSpeed
summary: Multiplier on how fast a subterranean unit sinks and rises, and divisor of its dig-in wait.
see_also: [ROT, MovementZone, AllowShroudedSubteranneanMoves]
when_omitted:
  kind: value
  value: "1"
---

Two parts of a dig read this value. A unit whose [`Locomotor`](/keys/locomotor/) tunnels first turns to face its destination. It then tips nose down for `(64 ÷ ROT) ÷ TunnelSpeed` frames before the descent begins, so a larger figure shortens that pause. The descent and the ascent then move it its full travel speed times this value, in leptons of height, each frame.

Nothing else in the dig reads it. A unit under the ground advances a fixed 19 leptons a frame toward its destination however this is set. The wait while it rights itself on the surface is `64 ÷ ROT` frames undivided. Raising the value shortens the dig-in and the two vertical legs and leaves the length of the underground crossing exactly as it was.

:::caution[The vertical rate has a floor]
A descent or ascent of fewer than five leptons a frame is raised to five. Lowering this value therefore cannot stall a dig, and once a unit's travel speed times the value falls under that floor, lowering it further changes nothing. `0` is the exception: the dig-in timer divides by this figure, so a zero divides by zero.
:::
