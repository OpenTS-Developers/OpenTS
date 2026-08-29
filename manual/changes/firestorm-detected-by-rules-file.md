---
title: Detect Firestorm from its rules file alone
category: fix
release: 0.2.0
targets:
- type: format
  id: rules-registries
  effect: changed
breaking: true
migration:
- Rename or remove a FIRESTRM.INI that a deployment ships without the rest of the expansion, since that
  file alone now switches the expansion on.
credit:
- ZivDero
---

Firestorm now counts as installed wherever `FIRESTRM.INI` is found. `EXPAND01.MIX` was
required alongside it, so a deployment holding the expansion's content in archives of its
own, or under names of its own, was played as the base game however complete it was: the
expansion's rules were not read, its game type was not offered, and its sounds, side
archives and map packet were passed over.

The file is looked for the way every other file is, so it may sit loose in any searched
folder or inside an archive.
