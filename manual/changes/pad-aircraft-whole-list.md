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

The share of a pad's price that stands for its aircraft is now the average of every `PadAircraft` entry. An empty list bundles no price and leaves a hoverpad without its free aircraft. The engine read entries 0 and 1 whatever the list held. An empty list was therefore read past its end the first time a building price was worked out, and the game crashed before a match could start.
