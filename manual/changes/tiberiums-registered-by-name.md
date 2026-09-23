---
title: Register Tiberium types by name
category: fix
release: 0.2.0
targets:
- type: format
  id: rules-registries
  effect: changed
- type: system
  id: tiberium
  effect: changed
credit: [ZivDero]
---

`[Tiberiums]` registers types by name, like the other rules lists, and the number on an entry no longer selects a slot. A map or any later rules file can now change a Tiberium type by writing its section, without listing it in `[Tiberiums]`. An entry that would register a fifth type is skipped instead of overrunning the four storage compartments.
