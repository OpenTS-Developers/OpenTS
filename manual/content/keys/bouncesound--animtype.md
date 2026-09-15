---
key: BounceSound
scope: animtype
label: Animation bounce sound
see_also: ["BounceAnim", "ExpireSound", "Elasticity", "Report"]
when_omitted:
  kind: value
  value: none
---

The sound plays at the thrown animation's position on the frame the bounce physics report a strike, under exactly the conditions [`BounceAnim`](/keys/bounceanim/#scope-animtype) describes. It plays on the one contact that ends the flight, not on a contact that settles the animation instead, and without regard to whether the ground under it is water.

A value naming no registered sound leaves whatever was set before in place rather than clearing it, so a misspelled name is silently ignored.
