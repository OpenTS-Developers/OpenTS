---
key: MaxVeinholeGrowth
summary: Ceiling on the veins one veinhole monster may cover.
see_also: ["system:veins", "VeinholeGrowthRate", "VeinGrowthEnabled"]
when_omitted:
  kind: value
  value: "1000"
---

Every monster is measured against the figure twice before each [growth step](/systems/veins/#growth): it may have handed out at most this many less 40 frontier entries, and it may cover at most this many less 100 mature cells. Neither test is ever relaxed, so the figure is a hard ceiling on one monster's field rather than a target the field settles at. Every monster in the scenario shares the one setting.

:::caution[A figure below 100 stops growth outright]
The coverage test compares a monster's mature-cell count against the figure less 100. A monster covering nothing at all already fails that test when the figure is below 100, so no vein grows anywhere in the scenario.
:::
