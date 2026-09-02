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

Compressed games start at a two-frame send period with six frames of look-ahead,
then calibrate from every player's process time and worst local round trip.
Early reports can select the measured target after 64 frames; incomplete
calibration falls back to `3/9` after 128 frames.

Worsening applies immediately. Recovery needs sustained headroom for its first
step and then continues one rung per evaluation; timing decreases drain the old
scheduling horizon before stepping down on aligned send boundaries. An event
scheduled for a frame that a decrease skips executes on the next send frame,
and a player whose measured RTT lapses holds the current timing instead of
selecting `10/250`. Reports also carry each player's longest wait for the
others, and improvement waits until nobody has waited 0.1 s or longer. The
inherited per-frame slowdown for a lagging player is removed; at adaptive send
periods it ran on every frame.

The disabled WOL Connection slider shows the effective 1–10 rung and tier; the
message list announces target-tier changes. Game speed remains separate.
`LATENCYFUDGE` stays in the replay layout but is no longer emitted or used by
the adaptive policy.

`NETWORK_REPORT` extends network events and multiplayer recordings. Players and
recordings therefore require the same OpenTS snapshot; existing event IDs are
unchanged and no configuration migration is needed.
