---
key: HomingScatter
summary: Parsed scatter distance that the engine never uses.
no_effect: true
see_also: ["BallisticScatter", "Inaccurate", "ROT"]
when_omitted:
  kind: value
  value: "2"
---

Nothing scatters a homing projectile. A projectile steered by the homing flight model, one whose [`ROT`](/keys/rot/#scope-bullettype) is above zero, flies at the aim point it was given. An [`Inaccurate=yes`](/keys/inaccurate/) setting on its type does not displace that point.

The one scatter a shot does receive belongs to an arcing projectile, one whose type sets [`Arcing=yes`](/keys/arcing/), and only when it is fired inaccurately; it is measured with [`BallisticScatter`](/keys/ballisticscatter/).
