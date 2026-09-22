---
title: Pace a game played alone by a frame rate for each speed
category: feature
release: 0.2.0
targets:
- type: key
  id: GameSpeed
  effect: changed
credit:
- ZivDero
- Rampastring
---

A single player mission or a skirmish is now held to a frame rate at each speed setting: 60 frames a second at Faster, 45 at Fast, 30 at Medium, 20 at Slow, 15 at Slower and 10 at Slowest. Fastest still runs as fast as the machine manages. Faster used to run at 62.5 and Slowest at 10.4, so both barely move. Fast goes from 31.25 to 45, and Medium, Slow and Slower each run close to what the next faster setting used to, so Medium, the default, goes from 20.8 frames a second to 30. Missions count frames, so only how long they take by the clock changes.

A network game keeps its own rates, which are slower at every setting but Slowest: 60 at Fastest, 45 at Faster, 30 at Fast, 20 at Medium, 15 at Slow, 12 at Slower and 10 at Slowest.

A network game left in the background no longer keeps a processor core busy between frames.

Rampastring is credited for the ts-patches change whose single-player speed table this uses.
