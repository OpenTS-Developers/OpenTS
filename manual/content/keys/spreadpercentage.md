---
key: SpreadPercentage
summary: Share of a Tiberium type's queued cells that one spread pass may seed from.
see_also: ["system:tiberium", "Spread"]
when_omitted:
  kind: value
  value: "0.1"
  note: A tenth of the type's queued cells.
---

The queued cell count multiplied by this fraction, clamped to between 5 and 25, gives the pass its ceiling. The pass then draws a random figure from 1 up to that ceiling. Only a cell that finds a free neighbor counts against the ceiling, so a pass can walk far more of the queue than the figure suggests when most of the field is hemmed in.

At `0.00001` or below the pass exits before touching the queue, and below that figure no individual cell of the type passes its spread test either.
