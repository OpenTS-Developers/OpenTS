---
key: AIBaseSpacing
summary: Cells of clearance kept around a building when base ground is reserved and computer placements are searched.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "1"
---

Every building that is not a vehicle reserves its footprint grown by this many cells on each side as ground occupied by its owner. The computer's [placement search](/systems/ai-base-building/#choosing-a-spot) grows a candidate footprint by the same margin before testing it. The reservation is kept for every house, but each house's search reads its own reservations only: a site is accepted only where its grown footprint touches ground the house itself has reserved. A reservation by any other house is invisible to the search; it neither satisfies that test nor holds the computer back.

The margin constrains the first placement attempt only. The second attempt runs when the first has found no acceptable site, drops the margin, and places against the bare footprint; there is no third.
