---
key: Land
summary: The land type a cell reports while this overlay is on it.
see_also: ["system:walls-and-gates", "NoUseTileLandType", "Tiberium", "CliffBackImpassability"]
when_omitted:
  kind: value
  value: Clear
---

```ini title="rules.ini"
[GAWALL]
Land=Wall
```

An overlay replaces the [land type](/reference/enums/land-type/) its cell would otherwise take from the tile underneath, and that land type is what the movement cost table and the buildable test read. `Wall` and `Railroad` settle the cell's land in a single step, subject only to the [cliff-back rule](/keys/cliffbackimpassability/) that can force `Rock` beneath a sufficiently higher neighbor. Any other value survives only while the overlay keeps [`NoUseTileLandType=yes`](/keys/nousetilelandtype/); with that turned off, the tile's own land type wins instead.

Two rules rewrite the value. [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype) promotes a `Clear` setting to `Tiberium` as the section is read. An overlay that turns `NoUseTileLandType` off and leaves `Land` at `Clear` reports `Tiberium` on a cell where Tiberium is growing.

`Wall` is what puts a cell into the `[Wall]` movement-cost section, and a man set on fire refuses to run into a cell that reports it. No stock wall overlay actually sets it; they leave `Land` at `Clear`. It is not what makes an overlay a wall. That is [`Wall=yes`](/keys/wall/#scope-overlaytype), which is read separately and owns blocking, damage and connection behavior.
