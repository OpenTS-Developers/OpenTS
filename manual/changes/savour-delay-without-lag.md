---
title: End a network game after the same delay whatever the connection
category: fix
release: 0.3.0
targets:
- type: key
  id: SavourDelay
  effect: changed
credit:
- ZivDero
---

In a multiplayer game other than skirmish, the delay between a house being flagged to win, lose or blow up and that result taking effect was at least the number of frames the machines ran ahead of each other, so a game over a slow connection ended later. The delay is now `SavourDelay`, extended to end on a frame number divisible by ten, whatever the connection.
