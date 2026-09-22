---
title: Keep a solo game running while the window is away
category: feature
release: 0.2.0
targets:
- type: key
  id: SimulateWhileUnfocused
  effect: added
credit: [ZivDero, dkeeton]
---

`SimulateWhileUnfocused=yes` under `[Options]` in `sun.ini` keeps a campaign or skirmish running while another program has the focus, without sound or input.

Returning to a paused game is also quicker: the game now checks for the focus every 10 milliseconds instead of every 500.
