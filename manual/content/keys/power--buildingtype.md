---
key: Power
scope: buildingtype
label: Power output and drain
when_omitted:
  kind: value
  value: "0"
---

A positive value is the power the type supplies to its house. A negative value is drain of the same magnitude and leaves the type supplying nothing. The two are mutually exclusive on one type: a positive value clears any drain, and a negative one clears the output. There is no separate drain key.

```ini title="rules.ini"
[MYPOWR]  ; example power plant BuildingType
Power=100

[MYRADAR] ; example radar BuildingType that consumes 40
Power=-40
```

An upgrade adds its own value to the structure it is plugged into, so a turbine's output and a plug's drain both reach the house through their host. [The power balance](/systems/power/#how-the-balance-is-computed) covers how the two figures are tallied, and [what low power costs](/systems/power/#what-low-power-costs) covers what a shortfall does to the base.

A type with a positive value also shows the house's power readout. Selecting one of its instances draws the owning house's current output and drain over it, for a house allied to the local player or for a building the local player has infiltrated with a spy.
