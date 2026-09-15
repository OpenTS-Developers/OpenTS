---
title: Size AI production demand from the type registries
category: fix
release: 0.1.0
targets:
- type: system
  id: ai-team-production
  effect: changed
credit: [ZivDero]
---

Vehicle, infantry and aircraft production demand now grows with the corresponding type registry instead of indexing three 100-entry arrays. The existing type order, oldest-team choice and synchronized random draws remain unchanged. A type position outside its registry is ignored rather than indexing another type's storage.
