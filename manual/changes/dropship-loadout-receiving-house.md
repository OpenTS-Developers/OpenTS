---
title: Deliver a dropship from the loadout of the house it is sent for
category: fix
release: 0.3.0
targets:
- type: key
  id: StartingDropships
  effect: changed
- type: action
  id: TACTION_REINFORCEMENTS
  effect: changed
- type: action
  id: TACTION_REINFORCEMENTS_SPECIAL
  effect: changed
credit:
- ZivDero
---

A `DSHP` reinforcement used to check the player's loadout and take its units from the loadout of the house it was sent for, so one sent for another house while the player's loadout held units crashed the game. It now uses only the receiving house's loadout, and is refused when that loadout is empty.

`StartingDropships` now opens the loadout screen only in a campaign mission. A skirmish or multiplayer map that sets it no longer shows the screen, so its `DSHP` reinforcements are refused.

Without an aircraft type named `DSHP` in the rules, either reinforcement action used to stop a Debug build at a failed assertion or read invalid memory in a Release build. It now delivers the team as it would any other.
