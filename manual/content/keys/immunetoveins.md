---
key: ImmuneToVeins
summary: Exempts the object type from vein damage.
see_also: ["system:veins", "VeinDamage", "VeinAttack"]
when_omitted:
  kind: value
  value: "no"
---

The exemption is read twice. A [vein cell](/systems/veins/#standing-in-veins) skips an exempt object when it decides whether to start its attack, so an exempt object standing alone on mature veins never sets one off. The attack, once running for something else in the same cell, passes over the exempt object when it deals out its damage. The `VEIN_PROOF` veteran ability grants a single object the same exemption without the type with this flag.
