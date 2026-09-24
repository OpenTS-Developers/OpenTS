---
title: Let a second copy of the game run with -MULTIINSTANCE
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:multiple-instances
  effect: added
credit: [ZivDero, dkeeton]
---

The `-MULTIINSTANCE` launch option lets a copy of the game start while another copy is running, for example to play a network match against yourself on one machine. Without it, a second copy still brings the running copy's window to the front and exits.

dkeeton is credited for the ts-patches debugging change this follows.
