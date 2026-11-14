---
key: TiberiumExplosive
scope: scenarios
label: Scenario flag
no_effect: true
see_also: ["system:tiberium"]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

Nothing in the game reads this entry. To make a destroyed vehicle carrying Tiberium explode, set [`TiberiumExplosive`](/keys/tiberiumexplosive/#scope-global-rules) in `[CombatDamage]` instead.
