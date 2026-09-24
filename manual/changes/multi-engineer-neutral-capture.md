---
title: Let one engineer capture a Neutral structure under Multi Engineer
category: balance
release: 0.2.0
targets:
- type: system
  id: capture
  effect: changed
credit: [ZivDero, Rampastring]
---

With the multiplayer engineer option on, an engineer that reaches a `Capturable=yes` structure of the `Neutral` house now captures it at any strength, as it does with the option off. Before, the engineer damaged it like any other structure, so taking it needed several engineers. Structures of every other house still take engineer damage while they are above `ConditionRed`.
