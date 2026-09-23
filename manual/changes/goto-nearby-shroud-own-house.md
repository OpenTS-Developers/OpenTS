---
title: Search the team's own shroud in Goto nearby shroud
category: fix
release: 0.2.0
targets:
- type: mission
  id: TMISSION_GOTO_SHROUD
  effect: changed
credit: [ZivDero]
---

A team running Goto nearby shroud now heads for cells its own house has not uncovered. Before, it searched the shroud of the player at each machine: a campaign team scouted toward whatever the player had not seen, and in a network game each machine could send the team somewhere different and fall out of sync.
