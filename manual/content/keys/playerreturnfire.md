---
key: PlayerReturnFire
summary: Lets human-owned objects fire back automatically outside the guard missions.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

At `no`, a human-owned object that is not a building answers damage only while its current mission is Guard, Area Guard or Patrol. Damage taken while it is moving, attacking, harvesting or in any other mission is absorbed without a response. At `yes` that mission restriction is lifted and any human-owned object that clears [the remaining retaliation tests](/systems/target-selection/#retaliation) answers.

```ini title="rules.ini"
[CombatDamage]
PlayerReturnFire=yes
```

Buildings are exempt from that mission restriction, but the setting reaches them on a second path. A damaged human-owned building left idle acquires its attacker directly only at `yes`, unless the attacker is an aircraft, which it answers regardless. A computer house never reads the setting on either path.
