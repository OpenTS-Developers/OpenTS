---
key: GDIPowerTurbine
summary: Seeds the first side's PowerTurbine.
see_also: [PowerTurbine, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[General]
GDIPowerTurbine=GAPOWRUP
```

The named BuildingType becomes the first side's [`PowerTurbine`](/keys/powerturbine/) as each rules file that has the key sets it; a `PowerTurbine=` in that side's own section of the same file overrides it. It has no other effect.
