---
title: Score the economy column by what each player spent
category: fix
release: 0.2.0
targets:
- type: system
  id: multiplayer-score-screen
  effect: changed
credit:
- Rampastring
- ZivDero
---

The economy column measured what a player still had standing against everything that player ever had, so a player who was wiped out read zero however much they had built, and a player who lost nothing read full marks for building little. It now measures what each player spent against what the biggest spender spent. Vinifera reworked the figure the same way.

The bar figures also finish where they should. The columns animate two pixels at a time, and the longest bar's last frame stopped one step short of its own total, leaving a full economy reading 99 on the screen it was drawn to.
