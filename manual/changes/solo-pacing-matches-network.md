---
title: Hold every game speed to a frame rate
category: feature
release: 0.2.0
targets:
- type: key
  id: GameSpeed
  effect: changed
- type: key
  id: SimulateWhileUnfocused
  effect: changed
- type: system
  id: network-synchronization
  effect: changed
credit:
- ZivDero
- Rampastring
---

A single-player mission or a skirmish is now held to 60 frames a second at Faster, then 45, 30, 20, 15 and 10 at the slower speeds, with Fastest still unlimited. It used to run at 62.5 at Faster and at 31.25, 20.8, 15.6, 12.5 and 10.4 below it. Missions count frames, so only their length in real time changes.

Every game now holds its rate exactly. Each frame's wait used to be rounded down to whole milliseconds, so a network game at 60 ran at 62.5.

A network game, or a solo game kept running with `SimulateWhileUnfocused=yes`, now keeps its full rate in the background, minimized or not, without keeping a processor core busy. It used to wait an extra 10 milliseconds each frame, so a game at 45 frames a second fell to about 31.

Rampastring is credited for the ts-patches change whose single-player speed table this uses.
