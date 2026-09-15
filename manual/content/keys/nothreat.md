---
key: NoThreat
summary: Keeps an object that is in this mission from being picked as a target by an automatic scan.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

The setting lives in a mission's own section. The mission tested is the one the candidate is in, not the one the object doing the looking is in. A candidate in a mission marked this way is rejected near the top of the target gauntlet, before ownership, range, category or [`LegalTarget`](/keys/legaltarget/) are considered.

```ini title="rules.ini"
[Harmless]
NoThreat=yes
```

A target chosen by a player order, by a team script or by retaliation reaches the object regardless of the mission it is in.

One other thing reads the setting. A vehicle, soldier or aircraft that belongs to no team leaves a mission marked this way and returns to idle behavior as soon as damage from an identified attacker lands without destroying it. [`Zombie=yes`](/keys/zombie/) in the same section is what holds it there instead.
