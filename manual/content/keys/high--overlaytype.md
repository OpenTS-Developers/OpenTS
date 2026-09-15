---
key: High
scope: overlaytype
label: High obstacle
see_also: ["system:walls-and-gates"]
when_omitted:
  kind: value
  value: "no"
---

A projectile flying below height 100 detonates the moment it reaches a cell with this overlay, unless the projectile is itself [`High=yes`](/keys/high/#scope-bullettype). The projectile explodes on the overlay's cell rather than on its intended target, which is what stops a flat-firing weapon from shooting through a wall it has no line over.

The test reads the overlay only. It is independent of [`Wall=yes`](/keys/wall/#scope-overlaytype), of the overlay's damage state, and of who owns the cell: a friendly wall stops friendly fire exactly as an enemy one does.
