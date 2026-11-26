---
key: DamageToInfantry
summary: The damage infantry take from a cell of this Tiberium type.
when_omitted:
  kind: value
  value: "-1"
  note: A value of -1 means the type deals its `Power` divided by ten, rounded down, and at least 1.
---

The value is the damage infantry take each time they finish moving into a cell of this type. `-1` counts as unset and uses the type's [`Power`](/keys/power/#scope-tiberium) divided by ten. Any other value of 1 or less deals 1 damage.

Infantry whose type sets [`TiberiumProof=yes`](/keys/tiberiumproof/), or that have the Tiberium-proof [veteran ability](/systems/veterancy/#abilities), take none. [Tiberium](/systems/tiberium/#damage) covers how that damage is applied, including the visceroid a death can leave.
