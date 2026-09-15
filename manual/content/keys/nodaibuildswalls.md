---
key: NodAIBuildsWalls
summary: Seeds the second side's AIBuildsWalls.
see_also: [AIBuildsWalls, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "yes"
---

The value becomes the [`AIBuildsWalls`](/keys/aibuildswalls/#scope-side) of the second side in the rules' [`[Sides]`](/formats/rules-registries/) list, as each rules file sets it. An `AIBuildsWalls=` in that side's own section of the same file overrides it. The first side has no matching key in `[General]`, and nothing else reads this one.
