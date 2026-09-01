---
title: Serialize save games member by member
category: internal
release: 0.1.0
breaking: true
migration:
- Finish or abandon games in progress before changing release-cycle versions or development snapshots. No converter is provided, and snapshots within one active cycle have no interoperability promise.
targets:
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

A save game stores each object one named member at a time rather than as a copy of its
memory. Each class lists what it stores, one list serves both saving and loading, and
whatever the file cannot carry — artwork, caches, session bookkeeping — is rebuilt as the
object loads.

Save games written by the vanilla game or by another OpenTS release-cycle version no longer
load; the version stamp in the file header must match the running project version.
Presentation state is rebuilt rather than stored, so the power bar and radar animations,
the mouse cursor's shape, and an in-progress selection or building placement come back at
their resting values. Internet-game unit tallies are no longer carried across a save.
