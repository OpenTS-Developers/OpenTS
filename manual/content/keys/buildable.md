---
key: Buildable
summary: Allows buildings and new Tiberium growth on a land type.
see_also: ["system:tiberium", "AllowTiberium"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[Rough]
Buildable=yes
```

Two tests read the flag: whether a cell can take a building foundation, and whether a cell will [accept new Tiberium](/systems/tiberium/#spread). A land type left at `no` blocks both at once. Each land type keeps its own setting, and a land type whose section is absent from the rules is not read at all.
