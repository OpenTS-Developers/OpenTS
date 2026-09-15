---
key: MinZVel
scope: animtype
label: Animation launch speed
see_also: ["MaxXYVel", "Bouncer", "IsMeteor"]
when_omitted:
  kind: value
  value: "3.5"
---

Leptons per frame: 256 leptons to a cell, 15 frames to the second. A thrown animation goes up at this speed plus a whole number of leptons, so this is the slowest launch the type can produce.

The span that whole number comes from is the truncated value of `4.5` minus this setting, and that `4.5` is built in: an animation has no setting of its own to raise it with. The pick hands back a non-negative offset whichever side of zero that lands on, so the speeds always run upward from the setting and the band grows wider the further the setting sits from `4.5`. A setting of `20` launches at `20` through `34`; `40` launches at `40` through `74`; the default `3.5` leaves a span of one and launches at `3.5` exactly, every time.

A meteor takes the figure as its vertical speed outright, with no pick at all. The sign then decides the approach, as [`IsMeteor`](/keys/ismeteor/#scope-animtype) describes.

:::danger[A setting just above the default stops the game]
Any value above `3.5` and below `5.5` truncates the span to zero. The division faults, and the game stops the moment a [`Bouncer=yes`](/keys/bouncer/) animation of the type is created. A meteor never makes the pick and is unaffected, and so is an animation that neither flag throws.
:::
