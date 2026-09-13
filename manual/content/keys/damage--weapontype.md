---
key: Damage
scope: weapontype
label: Weapon damage
see_also: ["system:emp-pulse"]
when_omitted:
  kind: value
  value: "0"
---

Each shot the weapon fires carries this figure, scaled by the firer's firepower modifiers when the shot comes from ordinary combat, and the warhead then decides what the figure does on impact. An [`EMEffect=yes`](/keys/emeffect/) warhead never reaches damage processing at all and spends it as [the pulse duration in frames](/systems/emp-pulse/#firing-a-pulse) instead.
