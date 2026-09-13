---
title: Show the saving and loading notices as RmlUi documents
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The notices the game shows while it saves or loads, for a menu save, a quick save, a load from the load dialog and the multiplayer load countdown, are now RmlUi documents where no Win32 dialog is on screen, and a document notice is drawn before the work starts rather than at the next paint. A progress box raised over a Win32 dialog, as the map generator and the scenario transfer raise theirs, stays a Win32 box, and so does every notice while `LegacyDialogs=yes`.
