---
title: Initialize every building animation state
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, AlexB]
---

Every building type now begins with all six animation-control records initialized. The five states with an ordinary default still begin at frame zero with one frame and no rate, while the otherwise unused full state begins entirely at zero. The multiplayer synchronization checksum therefore no longer includes whatever bytes happened to occupy that record before construction.
