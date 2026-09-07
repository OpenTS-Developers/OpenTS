---
key: NodAIBuildsWalls
summary: Seeds the second side's AIBuildsWalls.
see_also: [AIBuildsWalls, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "yes"
---

The value becomes the second side's [`AIBuildsWalls`](/keys/aibuildswalls/#scope-side) as each rules file sets it; an `AIBuildsWalls=` in that side's own section of the same file overrides it. Nothing else reads the key.
