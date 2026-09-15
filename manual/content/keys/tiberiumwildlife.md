---
key: TiberiumWildlife
summary: How readily a generated map's tiberium fields are given creatures to guard them, as a figure from 0 to 100.
see_also: [Tiberium, TiberiumLayout, "system:tiberium"]
when_omitted:
  kind: value
  value: "0"
  note: Every field's creature budget comes out at nothing, so no creatures are placed.
---

Each of the map's tiberium fields is given a creature budget of its own as it is grown, and the figure is what that budget is scaled by. A field spends its budget as it grows. A cell is drawn for the first creature before growth begins, and each time growth reaches the drawn cell a creature is turned loose there and another cell is drawn from those still to come. The creatures are drawn from a fixed set of four IDs: `VISC_SML`, `VISC_LRG`, `JFISH` and `DOGGIE`. Stock rules name them the baby and adult visceroid, the tiberium floater and the tiberian fiend. The names are looked up in rules at the moment a creature is placed, and a miss is not checked. A rules file that defines none of the four has the generator index the unit or infantry table with `-1`, so a placement is undefined rather than skipped. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
TiberiumWildlife=30
```

Nothing is placed without the Firestorm addon, which zeroes every budget. The fields the players start on are exempt as well, so the creatures are only ever found on the fields spread across the map. The map generator dialog offers the setting as a check box that writes `30` or `0`; its randomize button can write any figure in the range.

:::caution[The budget is drawn in a way that skips the intended middle ground]
A field's budget is a draw of five outcomes, scaled by the figure and truncated. Two of those outcomes were meant to come out negative and be discarded, but the subtraction is done in unsigned arithmetic, so they wrap to about four thousand million instead of to `-2` and `-1`. Up to `50` that scales to a budget large enough to stop limiting anything, and creatures keep being placed for as long as the field goes on growing. Above `50` the scaled figure no longer fits a whole number and those two outcomes place nothing at all. The three surviving outcomes go the other way and truncate to no creatures at all until the figure reaches `50`, and to at most two even at `100`. The `30` the dialog's check box writes therefore never places a creature except through the two outcomes that wrapped.
:::
