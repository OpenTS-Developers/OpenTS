---
title: Draw UI documents with the dialog fonts
category: internal
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

A UI document names one of two font families. `dlgsys` is the bitmap font the game's own dialogs draw their captions and button text with, read from its pair of sheets; because its glyphs are shaded rather than flat, asking for text in a color moves the whole palette toward that color instead of tinting it. `dlg-sans` is the face the Win32 dialogs asked Windows for, in the TrueType version that scales cleanly.

Neither is shipped. The engine finds the bitmap sheets in the game's own art and the sans face on the machine, and where either is missing it uses the font that ships with OpenTS, so a document never has to name a fallback of its own.

That shipped font is now Arimo rather than Open Sans, and the migrated screens draw with `dlg-sans`, so their text looks different from before. The screens are being rebuilt to match the original dialogs, and this is the first step of it.
