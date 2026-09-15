---
title: Keep a launch file's loading screen through a restart and a save
category: fix
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit: [ZivDero]
---

The scenario now keeps the loading picture and bar position a launch file asked for. A mission restarted from within the game, or resumed from a save in a session with no launch file, therefore shows the same picture. The override used to live only in the session that read the launch file. Such a restart or resume fell back to the game's own backdrop. Where the launch file's picture was the only one the deployment had, it fell back to none at all.
