---
key: NodBaseDefenseCoefficient
summary: Seeds the second side's AIBaseDefenseCoefficient.
see_also: [AIBaseDefenseCoefficient, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "1"
---

The value becomes the second side's [`AIBaseDefenseCoefficient`](/keys/aibasedefensecoefficient/#scope-side) as each rules file sets it; an `AIBaseDefenseCoefficient=` in that side's own section of the same file overrides it. Nothing else reads the key.
