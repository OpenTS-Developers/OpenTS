---
key: AdvancedPowerPlant
scope: side
label: Side advanced power plant
see_also: [RegularPowerPlant, PowerTurbine, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The second side takes NodAdvancedPower as each rules file sets it; every other side names none.
---

```ini title="rules.ini"
[Nod]
AdvancedPowerPlant=NAAPWR
```

The BuildingType a computer house playing for this side inserts to cover a [power shortfall](/systems/ai-base-building/#power-and-money-interventions) once the buildings it owns meet the type's [`Prerequisite`](/keys/prerequisite/) list; before then it inserts its [`RegularPowerPlant`](/keys/regularpowerplant/#scope-side). A side that also names a [`PowerTurbine`](/keys/powerturbine/#scope-side) tries the turbine first. Writing `<none>` clears an inherited value.
