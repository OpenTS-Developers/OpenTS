---
key: Power
scope: tiberium
label: Tiberium damage
when_omitted:
  kind: value
  value: "0"
  note: A damage figure of zero.
---

The value is the damage this Tiberium deals, and all three paths that spend it apply their result through [`C4Warhead`](/keys/c4warhead/):

- An infantry object without [`TiberiumProof=yes`](/keys/tiberiumproof/) and without the Tiberium-proof veteran ability takes `Power / 10` on every cell of this Tiberium it steps into. The damage is rounded down and never below 1. It is forced, so armor class and [`Immune=yes`](/keys/immune/) do not soften it.
- A chain reaction consumes half the growth stacked in the cell and deals that many stages multiplied by this value at the cell. A result of zero creates no explosion animation and deals no damage; the consumed growth and neighboring reaction checks still proceed.
- With [`TiberiumExplosive=yes`](/keys/tiberiumexplosive/) in `[CombatDamage]`, a harvester destroyed while carrying Tiberium explodes over one and a half cells, unless the scenario grants harvesters immunity. The blast deals the sum, across every Tiberium type in its hold, of the amount carried multiplied by that type's `Power`.
