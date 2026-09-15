---
key: Veinhole
summary: Turns damage from any warhead with the flag into an attack the victim blames on a veinhole monster.
see_also: ["system:veins", "VeinholeWarhead", "Retaliate"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[VeinholeWH] ; the WarheadType named by [CombatDamage] VeinholeWarhead
Veinhole=yes
```

[Vein damage](/systems/veins/#standing-in-veins) names no attacking object, so without this flag there is nothing for the victim to shoot back at. The flag supplies one. When the damage arrives with no source, the attacker is resolved to the monster that owns the veins in the next cell on the victim's path (its own cell when the victim is standing still).

:::caution[The flag is read on every warhead, not only the vein one]
The retaliation test reads this flag before it checks whether the two houses are allied, whether the victim's mission allows retaliation, and whether the victim has a weapon to answer with. Any weapon whose warhead sets it therefore forces its victims to turn on the attacker, allies included, and objects with nothing to shoot with are put on the same footing.
:::

The flag also decides that the victim retaliates at all. A computer-owned object always does. A player-owned object does so while it has neither a target nor a movement order of its own; one already ordered to attack something ignores the veins entirely.
