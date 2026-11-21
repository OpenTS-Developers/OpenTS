---
title: Restart a mission from the scenario file as it was first loaded
category: fix
release: 0.3.0
targets:
- type: system
  id: campaign-progression
  effect: changed
- type: format
  id: opents-ini
  effect: changed
credit:
- ZivDero
---

Restarting a mission, or replaying it after a loss, now reads the copy of the scenario file kept when the mission loaded, even without `CarryScenarioFile=yes`. Before, without it, the restart read the file from disk again, so a file edited or replaced during the mission took effect. With it on, a new game started under the same scenario file name no longer reads the copy kept from an earlier game.
