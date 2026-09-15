---
title: Keep selected difficulty when restarting missions
category: fix
release: 0.1.0
targets:
- type: system
  id: difficulty
  effect: changed
credit: [ZivDero, Rampastring]
---

Restarting a campaign mission or standalone mission now keeps the difficulty assigned while the scenario's houses are read. Applying campaign carry-over state no longer overwrites the player's newly assigned handicap with the environment's older value. Global flags, money, inherited mission time, and campaign stage still carry over.
