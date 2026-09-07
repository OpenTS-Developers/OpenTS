---
key: GDIWallDefenseCoefficient
summary: Seeds the first side's AIWallDefenseCoefficient.
see_also: [AIWallDefenseCoefficient, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "3"
---

The value becomes the first side's [`AIWallDefenseCoefficient`](/keys/aiwalldefensecoefficient/#scope-side) as each rules file sets it; an `AIWallDefenseCoefficient=` in that side's own section of the same file overrides it. Nothing else reads the key.
