---
title: Hold the campaign difficulty setting to the settings it names
category: fix
release: 0.2.0
targets:
- type: key
  id: Difficulty
  effect: changed
credit: [ZivDero]
---

`Difficulty=` under `[Options]` in `sun.ini` is now held to the three campaign difficulties the game has, rather than to five. A value above the three is read as the hardest, and one below them as the easiest. A file edited by hand to name a fourth or fifth could start a mission whose computer difficulty fell below the easiest one. That mission then read the difficulty table from outside itself. A difficulty chosen in the game is unaffected.
