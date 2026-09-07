---
key: GDIPowerPlant
summary: Seeds the first side's RegularPowerPlant.
see_also: [RegularPowerPlant, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

The value becomes the first side's [`RegularPowerPlant`](/keys/regularpowerplant/#scope-side) as each rules file sets it; a `RegularPowerPlant=` in that side's own section of the same file overrides it. Nothing else reads the key.
