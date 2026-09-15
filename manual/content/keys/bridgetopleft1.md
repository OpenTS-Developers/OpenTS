---
key: BridgeTopLeft1
summary: Position within a bridge tile set of the piece that starts an east-west span at its top-left end.
see_also: [BridgeTopLeft2, BridgeBottomRight1, BridgeMiddle1]
when_omitted:
  kind: value
  value: "-1"
  note: The position lands two tiles before the set's first piece rather than on a piece of it.
---

An east-west span is found by this end. Whether an engineer is mending a bridge or a span is being brought down, the search accepts a cell at this position showing subtile `8` and then works east from it toward the far end. [`BridgeMiddle1`](/keys/bridgemiddle1/) covers how a position is measured.

An end piece has no condition ladder of its own. It is marked whole or damaged instead, and the mark is copied to every neighboring cell drawn from the same piece.
