---
title: Break ambush when the ambusher is discovered
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

An object sitting in ambush now hunts once it is discovered. The old test asked whether the house doing the discovering was computer-controlled, not whether the ambusher's own house was. An ambusher found by a human player therefore stayed put and never attacked.
