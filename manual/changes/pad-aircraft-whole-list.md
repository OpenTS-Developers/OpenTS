---
title: Price the pad aircraft from the whole list
category: fix
release: 0.2.0
targets:
- type: key
  id: PadAircraft
  effect: changed
credit: [ZivDero]
---

The share of a pad's price that stands for its aircraft is now the average of every
`PadAircraft` entry, and an empty list bundles no price and leaves a hoverpad without its free
aircraft. The engine read entries 0 and 1 whatever the list held, so an empty list was read
past its end on the first building price worked out, which crashed the game before a match
could start.
