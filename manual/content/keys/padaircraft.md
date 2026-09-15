---
key: PadAircraft
summary: The AircraftTypes bundled into the price of the pad they dock at.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: ""
---

The structure that entry 0 names as its first [`Dock=`](/keys/dock/) target treats the average of the entries' [`Cost=`](/keys/cost/#scope-aircrafttype) figures as a share already bundled into its own price. The price asked for the structure adds that share back, so buying it and selling it both work from the figure written for the structure. Its repair bill works from that figure less the share. Separately, a [`HoverPad=yes`](/keys/hoverpad/) structure that was built rather than captured receives one aircraft of entry 0 as it opens, docked and on guard. [`SeparateAircraft=yes`](/keys/separateaircraft/) removes both the bundled price and the free aircraft.

The two structures need not be the same one: the price is charged to whatever entry 0 docks at, and the aircraft is handed to whatever sets `HoverPad=yes`. Only one structure ever takes the bundled share, so a second pad costs its written figure whatever it docks.

An empty list bundles no price into any structure, and a hoverpad then opens without a free aircraft. An entry 0 with no `Dock=` bundles no price either.
