---
key: NodAdvancedPower
summary: Seeds the second side's AdvancedPowerPlant.
see_also: [AdvancedPowerPlant, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

The value becomes the second side's [`AdvancedPowerPlant`](/keys/advancedpowerplant/#scope-side) as each rules file sets it; an `AdvancedPowerPlant=` in that side's own section of the same file overrides it. Nothing else reads the key.
