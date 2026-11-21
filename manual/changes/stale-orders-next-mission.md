---
title: Keep the next campaign mission from quitting by itself
category: fix
release: 0.3.0
targets:
- type: system
  id: campaign-progression
  effect: changed
credit:
- ZivDero
---

Closing the game window while a finished campaign mission's end screens or replay prompt were showing did nothing at the time, but the next mission then quit by itself once it had lasted as long in game time as the one before it. The next mission now plays on. A close request on those screens is still ignored.
