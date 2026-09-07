---
key: GDIWallDefense
summary: Seeds the first side's AIWallDefense.
see_also: [AIWallDefense, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "6"
---

The value becomes the first side's [`AIWallDefense`](/keys/aiwalldefense/#scope-side) as each rules file sets it; an `AIWallDefense=` in that side's own section of the same file overrides it. Nothing else reads the key.
