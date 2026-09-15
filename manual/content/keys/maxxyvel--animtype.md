---
key: MaxXYVel
scope: animtype
label: Animation lateral speed
see_also: ["MinZVel", "Bouncer", "IsMeteor"]
when_omitted:
  kind: value
  value: "15"
---

Leptons per frame: 256 leptons to a cell, 15 frames to the second. The two horizontal components of a thrown animation's launch velocity are drawn independently: each is a random whole number taken modulo twice this figure, then reduced by the figure itself. A whole-number setting therefore gives speeds from minus the setting up to one lepton per frame short of it, in either axis, so the positive extreme is never reached.

A meteor draws both components from the same range and then mirrors the pair whenever `X` falls below `-Y`, which is what confines its approach to one half of the compass.

:::danger[A small setting stops the game]
The pick divides by the truncated whole number of twice this figure, so any value above `-0.5` and below `0.5`, `0` among them, makes that divisor zero. The division faults, and the game stops the moment a [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype) animation of the type is created. An animation that is neither never makes the pick and is unaffected.
:::
