---
title: Show the message boxes as RmlUi documents
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The game's message boxes are now RmlUi documents when no Win32 dialog is on screen, with the same buttons in the same slots, Enter answering with the default button and Escape with the second. A box raised over a Win32 dialog, as the options and network dialogs raise theirs, stays a Win32 box, and so does every box while `LegacyDialogs=yes`.
