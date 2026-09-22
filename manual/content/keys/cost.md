---
key: Cost
summary: The credit price of one object of this type, or the multiplier a country or a difficulty applies to every price it pays.
---

An object's price is the figure most credit amounts in the game are worked out from: what a factory charges, what a sale refunds, the score a kill adds, and the experience it awards. [`Points=`](/keys/points/) governs none of these.

## What a structure gives away

A structure that comes with something else has that thing's price taken out of its own to make a second, lower figure:

- a [`FreeUnit=`](/keys/freeunit/) grant deducts that unit's own price, and the result is held at `0` rather than allowed to go negative;
- the first structure in the [`Dock=`](/keys/dock/) list of the first [`PadAircraft=`](/keys/padaircraft/) entry deducts the average price of the whole `PadAircraft=` list, rounded down, unless [`SeparateAircraft=yes`](/keys/separateaircraft/) is set or the structure's own [`FreeUnit=`](/keys/freeunit/) is an aircraft.

Almost nothing uses that lower figure. The price asked for the structure adds both deductions straight back, so what a factory charges, what a sale refunds, and what a kill is worth in score and experience all come to the written number. A refinery written at `2000` that hands over a `1400` harvester is still bought, sold and killed as a `2000` structure.

The lower figure reaches three things. It prices a step of [repair](/systems/repair/), which is the only place a player sees it, so a structure that comes with something else is cheaper to repair than its written price implies. It is the figure the test for handing over the free unit compares the price paid against, which [`FreeUnit=`](/keys/freeunit/) covers. And it sizes how much resentment damage to the structure earns its attacker, so hitting a refinery that has handed over its harvester stirs up less anger than its written price suggests.

Where the deduction would take the figure below nothing it stops at nothing. The price then adds back only what was given away, so a structure priced at or under the unit it hands out is worth that unit's price rather than its own.

## What a house pays

The two fractional forms of this key are multipliers rather than prices. A [country](/keys/cost/#scope-housetype) sets one and a [difficulty](/keys/cost/#scope-difficulty-settings) sets another, both starting at `1`, and a house multiplies the price by:

- **In a skirmish or a multiplayer match**, its country's multiplier and its difficulty's together.
- **In a single-player campaign**, its difficulty's alone. The country's is not read at all, so a country priced above or below par plays a campaign at par.
