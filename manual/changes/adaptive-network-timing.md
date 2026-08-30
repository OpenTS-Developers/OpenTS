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

Compressed matches begin with a three-frame send period and nine-frame
look-ahead. Each timing report records process time and optional round-trip
time together. A player receives an initial grace period while no round-trip
sample exists; a later missing or expired established sample selects the most
conservative timing. Accepted departure removes the player's whole report.

Timing reductions drain the previous scheduling horizon, switch on a frame
shared by the old and new send periods, and then reduce temporary look-ahead by
one new send period at each send boundary. A new target safely replaces that
transition. A successor master inherits the synchronized target and exhausted
change budget, then starts with fresh improvement evidence and a cooldown.

Timing reports extend the network and multiplayer-recording event stream. Run
every player with the same OpenTS snapshot and play a recording with the
snapshot that created it. Existing event IDs and packet layouts are unchanged;
timing events carry the selected look-ahead directly, and fog of war adds no
offset. There is no configuration to migrate.
