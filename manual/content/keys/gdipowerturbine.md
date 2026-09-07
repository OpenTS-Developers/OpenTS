---
key: GDIPowerTurbine
summary: Seeds the first side's PowerTurbine.
see_also: [PowerTurbine, "system:ai-base-building"]
when_omitted:
  kind: value
  value: none
---

The value becomes the first side's [`PowerTurbine`](/keys/powerturbine/#scope-side) as each rules file sets it; a `PowerTurbine=` in that side's own section of the same file overrides it. Nothing else reads the key.
