---
key: GDIPowerPlant
summary: Seeds the first side's RegularPowerPlant.
see_also: [RegularPowerPlant, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[General]
GDIPowerPlant=GAPOWR
```

The named BuildingType becomes the first side's [`RegularPowerPlant`](/keys/regularpowerplant/) as each rules file that has the key sets it; a `RegularPowerPlant=` in that side's own section of the same file overrides it. It has no other effect.
