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

Each connection measures its own round trip and retry timing. Compressed games
start with a two-frame send period and six-frame look-ahead, then calibrate early
from the whole match. Complete data can select the measured target directly;
missing initial measurements fall back to a three-frame period and nine-frame look-ahead.

Reductions drain the old scheduling horizon and step down at aligned send
boundaries. A successor master inherits the synchronized target and change
cooldown before restarting improvement hysteresis. Later recovery remains
available for the whole match.

The disabled WOL Connection slider shows the synchronized Fast, Normal, Poor,
or Bad tier, and the message list announces tier changes. Game speed remains a
separate setting. Adaptive timing uses measured RTT directly; the legacy
`LATENCYFUDGE` event remains in the recording layout but is no longer emitted
or used by the adaptive policy.

Timing reports extend the network and multiplayer-recording event stream, so
players and recordings require the same OpenTS snapshot. Existing event IDs
and packet layouts are unchanged, fog of war adds no timing offset, and there
is no configuration to migrate.
