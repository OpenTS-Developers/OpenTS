---
key: DamageLevels
summary: The number of damage stages a wall overlay passes through before it is removed.
see_also: ["system:walls-and-gates", "Strength"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="art.ini"
[GAWALL]
DamageLevels=3
```

The engine reads the count from the overlay's section in `art.ini`. Each [landed hit](/systems/walls-and-gates/#whether-a-hit-lands) advances a wall cell by one stage, and the segment is removed once the stage reaches this count. At the default of `1` the first landed hit removes the segment outright, so a wall survives damage only when the value is above one. A segment with no [connections](/systems/walls-and-gates/#connection-frames) is removed one stage early, at the stage below this count.

Two other rules read the count. The [cascade](/systems/walls-and-gates/#stepping-through-the-stages) that spreads damage into the neighboring segments fires when a cell reaches the stage below this count, and only when the count is above `2`. And a wall cell whose stored stage is exactly this count is a hole that infantry walk through unopposed. The damage path never leaves that state behind, because it removes the segment at that stage. A map's own overlay data can write it directly.

Higher counts need matching artwork. The engine deletes an unconnected segment of a stock wall overlay that reaches a stage the shipped art does not cover, without any damage being applied. [The collapse table](/systems/walls-and-gates/#damage-stages-with-no-artwork) has the stages per overlay.
