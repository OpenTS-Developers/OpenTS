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

Each connection measures its own round trip and retry timing. Peers report
process time and optional RTT together, letting the deterministic master adapt
the shared command delay from the whole match. Compressed games start at a
three-frame send period and nine-frame look-ahead; missing established reports
select conservative timing.

Reductions drain the old scheduling horizon and step down at aligned send
boundaries. A successor master inherits the synchronized target and change
budget before restarting improvement hysteresis.

The disabled WOL Connection slider shows the synchronized Fast, Normal, Poor,
or Bad tier, and the message list announces tier changes. Game speed remains a
separate setting; the menu no longer sends manual `LATENCYFUDGE` changes, and
new matches start with a 1× RTT margin.

Timing reports extend the network and multiplayer-recording event stream, so
players and recordings require the same OpenTS snapshot. Existing event IDs
and packet layouts are unchanged, fog of war adds no timing offset, and there
is no configuration to migrate.
