---
title: Leave the focus pause out of the play time
category: fix
release: 0.3.0
targets:
- type: key
  id: SimulateWhileUnfocused
  effect: changed
credit:
- ZivDero
---

While a campaign mission or skirmish was paused because its window lost the focus, its play time kept running, so the score screen and the diplomacy screen counted the time spent in another program. The play time now stops during that pause, as it does while an in-game menu is open.
