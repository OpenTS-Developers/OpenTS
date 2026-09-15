---
key: Overrides
summary: Overlay that no other overlay may replace once it is on a cell.
see_also: [Land, Wall, Tiberium]
when_omitted:
  kind: value
  value: "no"
---

The flag is read on the overlay already standing in a cell, not on the one arriving. An overlay being laid down asks whether the cell is empty of overlay or holds one that does not set this flag; if the answer is no, the placement is refused and the cell keeps what it had. Everything else about the arriving overlay still happens: the object is discarded either way, and a [`CellAnim`](/keys/cellanim/) is created whether or not the cell accepted the overlay.

```ini title="rules.ini"
[MYPIPE]         ; example ground fixture that Tiberium must not bury
Overrides=yes
```

:::caution[The name reads backward]
Setting the flag does not let a type override others. It protects a type from being overridden, so the overlays it shuts out are the ones written afterward.
:::

The protection lasts only for the placements that happen during play. While the scenario is being set up the test is skipped entirely, so the overlays a map declares are laid down in the order the map gives them and a protected overlay can be replaced by a later one.
