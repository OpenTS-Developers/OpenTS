---
title: Break ambush when the ambusher is discovered
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

An object sitting in ambush now breaks cover and hunts once it is discovered, which is what
the mission is for. The test asked whether the house doing the discovering was computer
controlled rather than whether the ambusher's own house was, so an ambusher found by a human
player stayed put and never attacked.
