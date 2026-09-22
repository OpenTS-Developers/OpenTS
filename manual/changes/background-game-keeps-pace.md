---
title: Keep a game in the background at its full frame rate
category: fix
release: 0.2.0
targets:
- type: system
  id: network-synchronization
  effect: changed
- type: key
  id: SimulateWhileUnfocused
  effect: changed
credit: [ZivDero]
---

A network game whose window is in the background no longer pauses for ten milliseconds every frame. The pause came on top of the wait that paces each frame, so a machine at 45 frames a second fell to about 31 while another application held the focus, and a network match cannot run ahead of its slowest machine. A single player mission or a skirmish kept running with `SimulateWhileUnfocused=yes` had the same pause and is fixed with it.
