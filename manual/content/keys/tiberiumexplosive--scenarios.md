---
key: TiberiumExplosive
scope: scenarios
label: Scenario flag
no_effect: true
see_also: ["system:tiberium"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

The value is stored with the other scenario flags and never read. The spelling that makes a loaded harvester explode is the [`[CombatDamage]` entry](/keys/tiberiumexplosive/#scope-global-rules).
