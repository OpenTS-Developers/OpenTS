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

Network games measure every peer-to-peer path instead of letting the host's own
links stand for the whole match. Healthy links use their own retransmission
timers, and the synchronized command delay can return toward a more responsive
setting after a temporary slowdown clears.

Timing reports extend the network and multiplayer-recording event stream. Run
every player with the same OpenTS snapshot and play a recording with the
snapshot that created it; there is no configuration to migrate.
