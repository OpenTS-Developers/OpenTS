---
title: Let any factory run its production animation
category: feature
release: 0.2.0
targets:
- type: key
  id: ProductionAnim
  effect: changed
credit: [ZivDero, Rampastring]
---

`ProductionAnim=` now plays as the finished object leaves any factory, including a barracks and a vehicle factory without `WeaponsFactory=yes`. It used to play only on a construction yard, refinery, repair bay or weapons factory. A hospital or armory does not play it, since it produces nothing.
