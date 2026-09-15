---
key: CliffBackImpassability
summary: Mode that turns a cell standing a cliff step below a neighbor into rock.
see_also: [Land, NoUseTileLandType]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[General]
CliffBackImpassability=2
```

`0` skips the rule. `2` is the only value that changes anything: it replaces the cell's [land type](/reference/enums/land-type/) with `Rock`, which is what the movement cost table and the buildable test then read. Any other value runs the height test and then does nothing with the result. The setting is kept as a single signed byte, so a figure outside `-128` to `127` wraps before it is compared.

The test asks whether a neighbor stands at least a full cliff step (four height levels) above the cell. Six neighbors are examined, and the list is not the eight cells around it. The cells at `(0,-1)`, `(-1,0)`, `(+1,-1)`, `(-1,+1)` and `(+1,+1)` are tested, along with `(+2,+2)` two steps away, while the cells at `(+1,0)`, `(0,+1)` and `(-1,-1)` are never tested.

The rewrite is the last step of a cell's attribute pass, so it overrides whatever earlier rules decided. Which land types it may overwrite depends on how the cell reached that point. A cell with an overlay that reports `Wall` or `Railroad`, or one set to [`NoUseTileLandType=yes`](/keys/nousetilelandtype/), is rewritten whatever land type the overlay gave it. A cell whose tile turned out to hold no artwork at that subtile has already been reset to `Clear` and is rewritten too. On the ordinary path only `Clear`, `Water`, `Beach` and `Ice` are rewritten, so a cell already reporting `Road`, `Rough`, `Tiberium`, `Weeds`, `Tunnel`, `Wall` or `Railroad` from its own tile keeps what it had.
