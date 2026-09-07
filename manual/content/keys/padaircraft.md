---
key: PadAircraft
summary: The AircraftTypes bundled into the price of the pad they dock at.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

The structure that entry 0 names as its first [`Dock=`](/keys/dock/) target treats the average price of every entry as a bundled share of its own [`Cost=`](/keys/cost/#scope-aircrafttype). The price asked for the structure adds that share back, so what it costs to buy, what selling it refunds and what destroying it is worth are all the written figure; the share is stripped out of its repair bill alone. Separately, a [`HoverPad=yes`](/keys/hoverpad/) structure that was built rather than captured receives one aircraft of entry 0 as it opens, docked and on guard. [`SeparateAircraft=yes`](/keys/separateaircraft/) removes both the bundled price and the free aircraft.

The two structures need not be the same one: the price is charged to whatever entry 0 docks at, and the aircraft is handed to whatever carries `HoverPad=yes`.

An empty list bundles no price into any structure, and a hoverpad then opens without a free aircraft. An entry 0 with no `Dock=` bundles no price either.
