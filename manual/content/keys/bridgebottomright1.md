---
key: BridgeBottomRight1
summary: Position within a bridge tile set of the piece that closes an east-west span at its bottom-right end.
see_also: [BridgeBottomRight2, BridgeTopLeft1, BridgeMiddle1]
when_omitted:
  kind: value
  value: "-1"
  note: The position lands two tiles before the set's first piece rather than on a piece of it.
---

This is where an eastward repair stops. The repair steps a cell at a time from the [top-left end](/keys/bridgetopleft1/), or from a middle piece at subtile `5`, restoring middle sections as it goes. It finishes when it meets a cell at this position showing subtile `4`. A walk that has not met one within twenty-nine cells returns without laying the bridge deck back down. A collapse walks the same way but stops at the first end at this position or the first middle section it meets. [`BridgeMiddle1`](/keys/bridgemiddle1/) covers how a position is measured.

An end piece has no condition ladder of its own. It is marked whole or damaged instead, and the mark is copied to every neighboring cell drawn from the same piece. A set may give the end one piece or two. [`BridgeBottomRight2`](/keys/bridgebottomright2/) names the second position of the same end, and every test accepts either key without telling them apart.
