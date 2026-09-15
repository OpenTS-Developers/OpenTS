---
key: Height
scope: smudgetype
label: Smudge rows
see_also: ["Width", "Crater", "Burn"]
when_omitted:
  kind: value
  value: "1"
---

The figure is how many rows of cells the smudge covers.

```ini title="rules.ini"
[MYCRATER]     ; example two-by-two crater
Crater=yes
Width=2
Height=2
```

A smudge is placed only if every cell of the block it would cover passes the fit test. The test rejects a spot with a covered cell under **None of:**

- a ramp tile;
- a smudge already;
- an overlay;
- a building, unless the request draws from the multiple-cell group, which ignores buildings.

The covered cell must also be a [`Morphable=yes`](/keys/morphable/) tile. Only the origin cell is checked against the [playfield](/keys/size/) that `Size=` declares, so the rest of the block may reach past it.

The block is then stamped one cell at a time. Every cell draws the same first frame of the artwork, shifted back to the block's origin, so one image covers the block rather than a frame per cell.

The figure also sorts craters and scorches into two groups. Only a smudge whose columns and rows are both above one counts as a large one. A large blast draws from that group, a small blast from the one-by-one smudges. Once a small blast measures more than 48 pixels wide and more than 40 pixels tall the size test stops applying, so every smudge that can be placed at the spot is eligible. When nothing in the group a blast draws from fits there, the blast falls back on every smudge that does.
