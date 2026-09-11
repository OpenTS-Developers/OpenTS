---
title: Adapt multiplayer timing to every connection
category: performance
release: 0.2.0
targets:
- type: system
  id: network-synchronization
  effect: added
credit:
- ZivDero
---

Compressed games start at a two-frame send period with six frames of
look-ahead, then calibrate from every player's process time and worst local
round trip. A worsening takes effect at the evaluation that sees it. Recovery
needs sustained headroom and no player waiting 0.1 s or longer, and a decrease
drains the old scheduling horizon before it takes effect. The inherited
per-frame slowdown for a lagging player is gone; at adaptive send periods it
fired on every frame.

The disabled WOL Connection slider shows the effective rung, 1 to 10, and its
tier, and the message list announces a change of target tier. The Speed slider
still sets game speed. `LATENCYFUDGE` keeps its place in the replay layout, but
nothing emits it and the timing policy does not read it.

`NETWORK_REPORT` is a new network event and appears in multiplayer recordings,
so every player and every recording needs the same OpenTS snapshot. Existing
event IDs are unchanged, and no configuration migration is needed.
