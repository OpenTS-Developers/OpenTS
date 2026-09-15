---
key: NodAdvancedPower
summary: Seeds the second side's AdvancedPowerPlant.
see_also: [AdvancedPowerPlant, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

The value becomes the [`AdvancedPowerPlant`](/keys/advancedpowerplant/) of the second side in the rules' [`[Sides]`](/formats/rules-registries/) list, as each rules file sets it. An `AdvancedPowerPlant=` in that side's own section of the same file overrides it. The first side has no matching key in `[General]`, and nothing else reads this one.
