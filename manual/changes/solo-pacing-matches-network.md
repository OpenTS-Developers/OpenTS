---
title: Pace a game played alone at the rates a network game uses
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

A single player mission or a skirmish now runs at the frame rate its speed setting gives a network game: 45 frames a second at Faster, 30 at Fast, 20 at Medium, 15 at Slow, 12 at Slower and 10 at Slowest. Fastest still runs as fast as the machine manages. Faster used to run at 62.5 frames a second, so a game at that setting is now slower; every other setting moves by four percent or less. Missions count frames, so only how long they take by the clock changes.

A network game left in the background no longer keeps a processor core busy between frames.

Rampastring is credited for the ts-patches change that first paced single-player games like multiplayer ones. Its speed table is not carried.
