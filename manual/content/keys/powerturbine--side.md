---
key: PowerTurbine
scope: side
label: Side power turbine
see_also: [RegularPowerPlant, AIUseTurbineUpgradeProbability, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes GDIPowerTurbine as each rules file sets it; every other side names none.
---

```ini title="rules.ini"
[GDI]
PowerTurbine=GAPOWRUP
```

The upgrade a computer house playing for this side inserts to cover a [power shortfall](/systems/ai-base-building/#power-and-money-interventions) while it owns a [`RegularPowerPlant`](/keys/regularpowerplant/#scope-side) whose upgrade slots are not full and a draw against [`AIUseTurbineUpgradeProbability`](/keys/aiuseturbineupgradeprobability/) succeeds. A base the computer takes over also gains one node of this type for each turbine its plants already carry. Writing `<none>` clears an inherited value; a side that names no turbine skips the draw.
