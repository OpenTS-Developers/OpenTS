---
key: BridgeTopRight1
summary: Position within a bridge tile set of the piece that starts a north-south span at its top-right end.
see_also: [BridgeTopRight2, BridgeBottomLeft1, BridgeMiddle2]
when_omitted:
  kind: value
  value: "-1"
  note: The position lands two tiles before the set's first piece rather than on a piece of it.
---

A north-south span is found by this end. Whether an engineer is mending a bridge or a span is being brought down, the search accepts a cell at this position showing subtile `12` and then works south from it toward the far end. [`BridgeMiddle1`](/keys/bridgemiddle1/) covers how a position is measured.

An end piece has no condition ladder of its own. It is marked whole or damaged instead, and the mark is copied to every neighboring cell drawn from the same piece.
