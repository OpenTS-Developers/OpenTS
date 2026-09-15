---
title: Allow zero-power Tiberium chain reactions
category: fix
release: 0.1.0
targets:
- type: key
  id: Power
  scope: tiberium
  effect: changed
credit: [ZivDero]
---

In `rules.ini`, a chain-reactive Tiberium type with `Power=0` now consumes the triggered growth and checks its neighbors without constructing a missing explosion animation. The reaction deals no damage, preserving zero as the configured value instead of silently raising it to one.
