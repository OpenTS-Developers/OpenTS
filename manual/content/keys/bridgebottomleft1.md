---
key: BridgeBottomLeft1
summary: Position within a bridge tile set of the piece that closes a north-south span at its bottom-left end.
see_also: [BridgeBottomLeft2, BridgeTopRight1, BridgeMiddle2]
when_omitted:
  kind: value
  value: "-1"
  note: The position lands two tiles before the set's first piece rather than on a piece of it.
---

This is where a southward repair stops. The repair steps a cell at a time from the [top-right end](/keys/bridgetopright1/), or from a middle piece at subtile `7`, restoring middle sections as it goes. It finishes when it meets a cell at this position showing subtile `2`. A walk that has not met one within twenty-nine cells returns without laying the bridge deck back down. A collapse walks the same way but stops at the first end at this position or the first middle section it meets. [`BridgeMiddle1`](/keys/bridgemiddle1/) covers how a position is measured.

An end piece has no condition ladder of its own. It is marked whole or damaged instead, and the mark is copied to every neighboring cell drawn from the same piece. A set may give the end one piece or two. [`BridgeBottomLeft2`](/keys/bridgebottomleft2/) names the second position of the same end, and every test accepts either key without telling them apart.
