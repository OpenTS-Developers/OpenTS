---
key: CloakDetectionRadius
summary: The distance in cells out from a flying jumpjet at which hidden objects are forced back into view.
see_also: ["system:cloaking"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[JumpjetControls]
CloakDetectionRadius=3
```

Every frame a jumpjet is moving, it sweeps a square this many cells out from the position it has just moved to and uncloaks everything standing in it. A figure of `2` covers a five-by-five block of cells. The engine default of `0` reduces the sweep to the jumpjet's own cell, so the setting must be raised before a jumpjet reveals anything beyond the cell it is directly over.

:::caution[The sweep ignores ownership entirely]
Nothing in it tests the house. A jumpjet flying over its own base uncloaks its owner's hidden vehicles and structures, and its allies', exactly as it uncloaks an enemy's. [Losing a cloak](/systems/cloaking/#losing-a-cloak) lists what that costs the objects caught underneath.
:::
