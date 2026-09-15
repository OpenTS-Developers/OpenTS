---
key: GrowthPercentage
summary: Share of a Tiberium type's queued cells that one growth pass may ripen.
see_also: ["system:tiberium", "Growth"]
when_omitted:
  kind: value
  value: "0.1"
  note: A tenth of the type's queued cells.
---

The queued cell count multiplied by this fraction, clamped to between 5 and 50, gives the pass its ceiling. The pass then draws a random figure from 1 up to that ceiling and takes that many cells. The clamp means a pass always draws at least one cell and never more than 50, however large or small the field is.

At `0.00001` or below the pass exits before touching the queue, and below that figure no individual cell of the type passes its growth test either.
