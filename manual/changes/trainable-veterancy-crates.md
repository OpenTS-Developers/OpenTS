---
title: Honor Trainable for veterancy crates
category: fix
release: 0.1.0
targets:
- type: system
  id: veterancy
  effect: changed
- type: key
  id: Trainable
  effect: changed
credit: [ZivDero, Iran, CCHyper]
---

A veterancy crate now promotes only objects whose types have `Trainable=yes`. It promoted every object within its radius before, and still promotes them whoever owns them.
