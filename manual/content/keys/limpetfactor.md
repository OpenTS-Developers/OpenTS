---
key: LimpetFactor
summary: The percentage of a vehicle's speed a limpet attachment takes away.
see_also: [ROT]
when_omitted:
  kind: value
  value: "0"
---

Any figure of `1` or more turns the weapon into a limpet attachment instead of a shot whenever the target is a unit, an infantryman, an aircraft or a building rather than a cell. The firing object creates no projectile: it marks the target with its own house, sets the target's speed multiplier to the remaining fraction, plays the weapon's sound, springs the limpet trigger event on the target, and removes itself.

```ini title="rules.ini"
[MyLimpetWH] ; example WarheadType
LimpetFactor=50 ; the target keeps half its speed
```

A target may hold one mark from each house. A shot at a target its own house has already marked does nothing at all: no projectile, no second mark, and the firing object survives. In the shipped rules the object that fires is the deployed limpet mine, a structure the limpet drone folds into; the drone itself has no weapon.

A mark changes three things about the object holding it. Only a vehicle is slowed, because the multiplier is read where a vehicle works out its top speed and nowhere else; infantry, aircraft and buildings hold the mark at full speed. An object holding the local player's own mark reveals ground for that player instead of for its owner, which is what makes the attachment worth firing. An aircraft is exempt, because it works out its own sight and never reads the mark. Its selection marker changes as well (a marked building's box is drawn in yellow), and the repair cursor is offered over a marked building whatever its strength.

Healing removes it. Any damage below zero clears the mark and restores the object's [`ROT`](/keys/rot/#scope-aircrafttype), so a repair weapon undoes an attachment outright. A repair step inside a service depot clears it the same way, and clears it before the vehicle's strength is looked at.

:::caution[The value is stored as a whole number]
The figure is truncated to a whole number before it is stored, so anything below `1` is stored as `0` and attaches nothing. That includes the percentage form used by the warhead's other fractional settings: `LimpetFactor=50%` is read as one half, truncated to zero, and the weapon fires an ordinary shot.
:::

:::caution[The turn rate is never slowed]
The rate of turn is scaled on the firing object rather than on the target: the firing object's own `ROT` is multiplied by the factor just written onto the target, and it is removed on the same step. A marked vehicle turns its body and its turret at full speed however low this setting goes.
:::

:::danger[A figure above 100 speeds the target up]
The shortfall below 100 is worked out as an unsigned quantity, so `LimpetFactor=150` does not reverse or freeze the target. It yields a multiplier of about 43 million: a marked vehicle's top speed is scaled by that figure rather than reduced. Exactly `100` is the value that stops a vehicle dead.
:::
