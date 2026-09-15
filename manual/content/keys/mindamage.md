---
key: MinDamage
summary: The smallest damage a warhead may deal to a target close to the impact.
when_omitted:
  kind: value
  value: "1"
---

Damage is scaled by the warhead's [`Verses`](/keys/verses/) percentage against the target's armor and then divided by [distance from the impact](/systems/warheads/#how-distance-thins-the-damage). This value is the floor the result is raised to, but only while that divisor is below 4. Past that the shot is allowed to fall away to nothing. The figure is then capped by [`MaxDamage`](/keys/maxdamage/).

```ini title="rules.ini"
[CombatDamage]
MinDamage=10
```

:::caution[The floor also lifts the low-power damage tick]
The [structure damage tick](/systems/power/#the-structure-damage-tick) enters this path with a raw damage of 1 and an impact distance of zero, so it comes out at whatever this value holds. Setting it to 10 makes a shortfall cost ten strength per tick instead of one.
:::
