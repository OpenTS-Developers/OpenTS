---
title: Play a game against other machines from a client's launch file
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit: [ZivDero]
---

A launch file describing a game against other machines now plays it. The match is
assembled whole from the file — the options, the seats, the alliances, the start
positions and the addresses — and carried over the network the file names: through a
CnCNet tunnel when one is given, where every machine is known by its tunnel number, or
straight between the machines at the addresses they wrote for one another.

Such a match is held to two rules a skirmish is not: every person must be named, and no
two may be named the same. The seat order the machines have to agree on is settled by
color and then by name, so a match missing those names is not one match. Sharing a color
is still allowed.
