---
title: Launch a campaign mission from a client's launch file
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit: [ZivDero]
---

A launch file marked as a single-player game now starts the mission it names. The campaign
the mission belongs to, the two difficulties, and the scenario flags a client carries over
from an earlier mission all reach the game's own state before the mission is read, so a
mission launched partway through a chain begins in the state the missions before it left.

The two difficulties are named apart, so a client may combine all nine pairings where the
menu offers its three coupled ones. To make that possible the pair became session state:
whichever path starts a campaign sets it, the menu deriving the same pair the mission
reader used to compute for itself, and the mission reader now takes the pair from the
session. A restart or the next mission keeps it, as before.

A dead difficulty table fell out of the campaign menu on the way. It was overwritten by the
mission reader on every start, and two of its five cases answered settings the game's
three-setting slider cannot reach. Live behavior is unchanged.
