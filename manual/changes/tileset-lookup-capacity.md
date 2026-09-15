---
title: Remove the theater tile-set lookup ceiling
category: fix
release: 0.1.0
targets:
- type: format
  id: theater-control
  effect: changed
credit: [ZivDero]
---

The theater loader now owns one checked lookup entry for every contiguous numbered tile set instead of writing past a 255-entry array. Marble-madness alternates are resolved only when both their target set and their tile offset exist. A malformed reference becomes no alternate and is reported, instead of corrupting the bridge-role values that follow the old array.
