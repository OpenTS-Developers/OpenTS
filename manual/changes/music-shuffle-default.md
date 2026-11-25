---
title: Pick the next music track at random by default
category: feature
release: 0.3.0
targets:
- type: key
  id: IsScoreShuffle
  effect: changed
credit:
- JoyfulShush
---

The game now picks each next music track at random when `sun.ini` leaves `[Audio] IsScoreShuffle` out; it used to take the tracks in THEME.INI order. The sound options screen shows Shuffle ticked in that case, and `IsScoreShuffle=no` keeps the list order.
