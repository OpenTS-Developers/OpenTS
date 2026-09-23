---
title: Count overlapping cloak and sensor fields
category: fix
release: 0.2.0
targets:
- type: system
  id: cloaking
  effect: changed
- type: key
  id: SensorArray
  effect: changed
- type: key
  id: CloakGenerator
  effect: changed
credit: [ZivDero]
---

A cell covered by two cloak generators or two sensor arrays of one house now stays covered as long as one of them still covers it. Before, a cloaked cell lost its cover until the other generator's field grew back over it, and a sensed cell stayed sensed only if the other array was operational at that moment.

Capturing a sensor array now moves its coverage to the new owner. Before, the old owner kept sensing the array's cells for the rest of the game, and the new owner sensed nothing until a cloak field finished growing or another array was removed.
