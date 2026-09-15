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

The upgrade a computer house playing for this side inserts to cover a [power shortfall](/systems/ai-base-building/#power-and-money-interventions). It qualifies only while the house owns a [`RegularPowerPlant`](/keys/regularpowerplant/) with a free upgrade slot and a draw against [`AIUseTurbineUpgradeProbability`](/keys/aiuseturbineupgradeprobability/) succeeds. When the computer takes a base over, the plan also gains up to one node of this type for each turbine slot the base's regular power plants already fill, set where the plan still runs short of power. Writing `<none>` clears an inherited value; a side that names no turbine skips the draw.
