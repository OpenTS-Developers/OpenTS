---
key: Sticky
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: ["Damage"]
when_omitted:
  kind: value
  value: "no"
---

The name promises that the animation pins itself to whatever unit is standing where it appears, which is how an impact explosion would ride the vehicle it went off on. Nothing reads the flag.

Whether an animation ends up pinned to an object is settled by the code that creates it, never by the animation's own type. The code lowering a parachute pins it to what it lowers, the fire an object catches is pinned to that object, and the sparks an EM pulse leaves are pinned to what they disabled.
