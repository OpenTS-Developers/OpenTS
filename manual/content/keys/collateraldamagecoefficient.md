---
key: CollateralDamageCoefficient
summary: The fraction of its own maximum strength a destroyed object deals to its surroundings.
see_also: [Explodes, Strength, ExpSpread, Cyborg, Storage]
when_omitted:
  kind: context-dependent
  note: "`1` for an AircraftType, BuildingType or UnitType and `0.66` for an InfantryType. A `Cyborg=yes` InfantryType falls to `0.33`, but only from the second rules layer that names its section onward, because the cyborg flag is not yet set when the value is chosen on the first pass."
---

```ini title="rules.ini"
[MYAMMOTRUCK] ; a UnitType registered in [VehicleTypes]
Explodes=yes
Strength=200
CollateralDamageCoefficient=2.5 ; 500 damage delivered on death
```

The coefficient multiplies the type's [`Strength`](/keys/strength/#scope-aircrafttype), its full maximum rather than the strength it had left. The product is the damage a destroyed object deals to everything around it. A structure adds the [`Power`](/keys/power/) of each Tiberium held in its [`Storage`](/keys/storage/) on top, which is why a full refinery makes a far larger crater than an empty one.

That figure is reached only on a death that explodes, so the setting does nothing at all unless the type is [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) or the object holds the veteran explodes ability. It also sizes the blast radius, and because the radius calculation divides the figure by 100 and discards the remainder, any product below 100 collapses to the minimum radius.

:::caution[A later rules file resets this key]
The value is re-derived from the object kind immediately before it is read, so a later rules layer that names the type's section, for any key at all, discards a coefficient an earlier file set. Repeat the assignment in every file that declares the section.
:::
