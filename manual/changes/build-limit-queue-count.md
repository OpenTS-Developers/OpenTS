---
title: Let the queue fill a positive build limit
category: fix
release: 0.2.0
targets:
- type: system
  id: production
  effect: changed
- type: key
  id: BuildLimit
  effect: changed
credit: [ZivDero, Rampastring]
---

A player can now queue a type with a positive `BuildLimit` up to the limit while one is in production. The object under construction used to count twice, once among the objects owned and once in production, so the queue refused an order one object early. With `BuildLimit=3` and one in production, only one more could be queued; now two can.

Rampastring is credited for the ts-patches fix this follows.
