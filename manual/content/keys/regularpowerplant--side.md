---
key: RegularPowerPlant
scope: side
label: Side power plant
see_also: [AdvancedPowerPlant, PowerTurbine, BuildPower, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes GDIPowerPlant and the second NodRegularPower, as each rules file sets them; any other side names no plant and inserts the first BuildPower entry the house's country may own.
---

```ini title="rules.ini"
[GDI] ; the section carrying the side's own name
RegularPowerPlant=GAPOWR
```

The BuildingType a computer house playing for this side inserts to cover a [power shortfall](/systems/ai-base-building/#power-and-money-interventions) when neither its [`PowerTurbine`](/keys/powerturbine/#scope-side) nor its [`AdvancedPowerPlant`](/keys/advancedpowerplant/#scope-side) is taken. It is also the plant whose free upgrade slot qualifies the house for the turbine, and the plant a house taken over by the computer gains turbine nodes on. Writing `<none>` clears an inherited value, and a side with no plant at all falls back to the first [`BuildPower`](/keys/buildpower/) entry the house's country may own.

The section shares its name with the side, so for the stock rules it is the same `[GDI]` and `[Nod]` sections that describe the countries of those names.
