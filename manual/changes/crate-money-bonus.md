---
title: Set the random extra money a crate pays
category: feature
release: 0.2.0
targets:
- type: key
  id: CrateMoneyBonus
  effect: added
- type: system
  id: crates
  effect: changed
credit: [ZivDero, Rampastring]
---

`CrateMoneyBonus` in `[CrateRules]` of `rules.ini` sets the most credits a money crate adds at random to its configured amount. It defaults to `900`, the amount the game always added, and `0` makes a money crate pay its configured amount exactly.

Rampastring is credited for the DTA patch that paid money crates their exact amount.
