---
title: Count each mature vein cell once toward a veinhole's limit
category: fix
release: 0.2.0
targets:
- type: system
  id: veins
  effect: changed
- type: key
  id: MaxVeinholeGrowth
  effect: changed
credit:
- ZivDero
---

Loading a scenario counted a veinhole monster's mature vein cells twice toward its growth limit, `MaxVeinholeGrowth` in `[General]` of `rules.ini`, so a saved field of about 900 mature cells stopped growing. Each mature cell now counts once, and a cell that fails to mature no longer counts.
