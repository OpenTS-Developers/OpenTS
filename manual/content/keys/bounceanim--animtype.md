---
key: BounceAnim
scope: animtype
label: Animation bounce effect
see_also: ["BounceSound", "Elasticity", "DamageRadius", "ExpireAnim"]
when_omitted:
  kind: value
  value: none
---

An animation of the named type is created at the thrown animation's position on the frame the bounce physics report a strike. A strike is contact with the ground, a bridge deck, or a building or wall standing in the way. A contact that leaves too little motion to keep tracking is reported as settling rather than as a strike, and plays nothing. [`Elasticity`](/keys/elasticity/#scope-animtype) decides which of the two a landing is, and every shipped bouncing animation sets a value that makes an ordinary landing settle.

The strike is also the end of the flight. The animation is removed on the same frame, so this effect plays at most once and alongside [`ExpireAnim`](/keys/expireanim/#scope-animtype), at the same place.

Nothing on this path tests what is underneath. An animation striking water plays this animation and its sound where it comes down. The expiry that follows on the same frame plays no blast there: a wake and a splash take its place.

A value naming no registered animation is not refused: a type of that name is created on the spot, holding no artwork and every setting at its built-in value.
