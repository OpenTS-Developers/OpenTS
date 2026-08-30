---
title: Assemble a launch against other machines up to its network
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit: [ZivDero]
---

A launch file describing a game against other machines is now read and judged like any
other, and the match it asks for is assembled whole — the options, the seats, the alliances,
the start positions, and the address each machine is reached on, whether directly or through
a tunnel. The launch is then refused at the network itself, which the game does not yet
open, with the reason shown.

Such a match is held to two rules a skirmish is not: every person must be named, and no two
may be named the same. The seat order the machines have to agree on is settled by color and
then by name, so a match missing those names is not one match. Sharing a color is still
allowed.
