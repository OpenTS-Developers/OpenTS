---
key: TiberiumGrowthEnabled
summary: Allows Tiberium to ripen and spread during a scenario.
see_also: ["system:tiberium", "TiberiumSpreads", "TiberiumGrows"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="map file"
[Basic]
TiberiumGrowthEnabled=no
```

With the switch off, neither the [growth pass nor the spread pass](/systems/tiberium/#growth) runs for any type, and the per-cell growth test fails as well. No queued cell then ripens or seeds, though blossom trees, crates, meteors and scattered loads still place Tiberium. The [Tiberium growth](/mapping/actions/taction-tib-growth/) trigger action writes the same switch while the scenario is running.
