---
key: Crater
scope: animtype
label: Animation crater
see_also: ["Scorch", "Flamer", "Craters", "CraterLevel"]
when_omitted:
  kind: value
  value: "no"
---

A crater is stamped where the animation stands, on the frame whose artwork covers the most ground. It is stamped only while the animation is standing less than 30 leptons above the ground beneath it. The crater is picked at random from the smudge types that declare themselves craters and fit the location. A location fits only where none of the following is true:

- the cell lies outside the playfield
- the ground is a ramp
- the cell already has a smudge of any kind
- the cell holds an overlay
- a building stands on the cell
- the theater's tile does not allow marks on it

Multiple cell craters join the candidates only where that largest frame measures more than 48 pixels across and 40 down, or where the rules declare no single cell crater at all.

Cratering also strips Tiberium from the cell, as [Damage](/systems/tiberium/#damage) explains. Both effects run each time the animation passes that frame, so a looping animation repeats them, though a cell already marked takes no second mark.

[`Scorch`](/keys/scorch/) takes precedence over this setting: an animation declaring both leaves a scorch mark half the time and a crater the other half, never both.

:::caution[A thrown animation rarely craters]
The frame test is skipped for as long as an animation is bouncing. A [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype) animation therefore marks the ground only when its largest frame happens to be its first. Even then it marks the point it was thrown from, not the point it comes down on. The terrain a meteor impact slumps is a separate effect that [`CraterLevel`](/keys/craterlevel/) covers.
:::
