---
title: Build next to a mutually allied base
category: feature
release: 0.2.0
targets:
- type: system
  id: base-adjacency
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
- type: key
  id: BuildOffAllyAnyStructure
  effect: added
breaking: false
credit:
- ZivDero
- Iran
- AlexB
- CCHyper
- tomsons26
---

`BuildOffAlly=Yes` admits a mutually allied house's buildings as placement anchors alongside
your own. The alliance must run both ways, so a one-sided declaration opens nothing, and an
anchor still needs `BaseNormal=yes`; an ally's walls and bibs count for nothing where your own
carry a wall placement. Only the placing machine runs the check, and a computer house builds
by a rule that never looks at an ally.

`[MultiplayerDefaults] BuildOffAllyAnyStructure` narrows which of the ally's buildings
qualify, from all of them to construction yards alone.

Iran is credited for the CnCNet spawner feature this follows, AlexB for the ts-patches patch
that corrected it, and CCHyper and tomsons26 for the Vinifera version.
