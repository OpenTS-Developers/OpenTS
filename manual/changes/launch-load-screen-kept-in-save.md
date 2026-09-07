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

The scenario now keeps the loading picture and bar position a launch file asked for, so a
mission restarted from within the game, or resumed from a save in a session that has no launch
file, shows the same picture. The override lived only in the session that read the launch
file, so such a restart or resume fell back to the game's own backdrop, or to none at all where
the launch file's picture was the only one the deployment had.
