---
key: BallisticScatter
summary: How far an inaccurate arcing shot has its aim point thrown off.
see_also: ["Inaccurate", "Arcing", "HomingScatter"]
when_omitted:
  kind: value
  value: "1"
---

The distance is written in cells, and one branch reads it. The aim point of a projectile whose type sets both [`Inaccurate=yes`](/keys/inaccurate/) and [`Arcing=yes`](/keys/arcing/) is moved by between half of this figure and all of it, in a direction picked at random over the whole circle. A shot is therefore never displaced by less than half the value. The key has no other reader: a projectile type with one flag without the other is fired without this scatter. [`Inaccurate`](/keys/inaccurate/) covers what the displacement does to the shot.

The scatter is applied to the offset from the barrel to the predicted target position, before the launch pitch is solved, so it changes the angle the shot leaves at as well as where it comes down.
