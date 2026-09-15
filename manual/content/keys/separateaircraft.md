---
key: SeparateAircraft
summary: Stops a helipad coming with a free aircraft and pricing itself around one.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "no"
---

At `yes` two things are switched off together. The structure that the first [`PadAircraft`](/keys/padaircraft/) entry docks at no longer strips the average price of the whole `PadAircraft` list out of its repair figure. It is repaired against the plain [`Cost=`](/keys/cost/#what-a-structure-gives-away) written for it instead. What it costs to buy and what selling it refunds are the written figure either way, because the price asked adds that same average straight back. The other change is that a [`HoverPad=yes`](/keys/hoverpad/) structure no longer receives a free copy of the first `PadAircraft` entry as it opens.

Nothing else changes: aircraft are still built at pads and still dock at them, and are now paid for one at a time.
