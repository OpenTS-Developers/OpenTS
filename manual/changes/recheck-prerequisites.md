---
title: Re-check buildability when the sidebar recalculates
category: feature
release: 0.2.0
targets:
- type: key
  id: RecheckPrerequisites
  effect: added
- type: system
  id: sidebar
  effect: changed
credit: [ZivDero]
---

`RecheckPrerequisites=yes` in `[General]` of `rules.ini` has the sidebar recheck tech level, prerequisites and ownership, so a cameo whose prerequisite is destroyed leaves the sidebar. Its production is cancelled: the item being built, every queued copy, and a structure waiting to be placed. Without the key a cameo stays once it has appeared.
