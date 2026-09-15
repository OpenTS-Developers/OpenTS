---
key: NodRegularPower
summary: Seeds the second side's RegularPowerPlant.
see_also: [RegularPowerPlant, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

The value becomes the [`RegularPowerPlant`](/keys/regularpowerplant/) of the second side in the rules' [`[Sides]`](/formats/rules-registries/) list, as each rules file sets it. A `RegularPowerPlant=` in that side's own section of the same file overrides it. [`GDIPowerPlant`](/keys/gdipowerplant/) does the same for the first side, and nothing else reads this key.
