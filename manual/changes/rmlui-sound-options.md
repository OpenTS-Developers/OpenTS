---
title: Show the sound options as an RmlUi document
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The sound options open as an RmlUi document from the options menu and from the in-game options, with the same three ten-step sliders and, in game, the same track list, Play, Stop, Shuffle and Repeat. Every change still takes effect at once and nothing is reverted on close; Escape closes the document the way OK does. `LegacyDialogs=yes` keeps the Win32 dialog.
