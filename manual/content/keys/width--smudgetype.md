---
key: Width
scope: smudgetype
label: Smudge width
see_also: ["Height", "Crater", "Burn"]
when_omitted:
  kind: value
  value: "1"
---

The figure is how many cells wide the smudge is. [`Height`](/keys/height/#scope-smudgetype) covers the block the two figures describe together: its placement test, the way one image is stamped across it, and the size grouping that decides which blast draws which smudge.

The width also numbers the cells of the block. Each covered cell records its position as the column plus the row times this figure. The draw, the terrain-flattening cleanup and the pavement-laying cleanup all divide that number back down to find the block's origin cell from any cell in it.

```ini title="rules.ini"
[MYCRATER]     ; example two-by-two crater
Crater=yes
Width=2
Height=2
```

:::caution[A figure below one places no smudge and still crowds out the ones that would]
The stamping loop runs once per column, so a figure of `0` or less writes to no cell at all. The type is not skipped, though: it passes the placement test for every location, because that test runs over the same empty loop. It then joins the pool a blast picks from. Picking it leaves no mark, and the mark a well-formed smudge would have left is not drawn instead.
:::
