---
title: Draw dialog text in the system's bitmap face
category: feature
release: 0.2.0
targets:
- type: key
  id: BitmapSystemFont
  effect: added
credit:
- ZivDero
---

The lists, text boxes, tooltips and hotkey fields of the migrated screens are drawn in the bitmap face the game's dialogs used, rather than in a scalable stand-in for it, so their letters match the dialogs beside them. `BitmapSystemFont` turns that off and leaves the scalable face drawing at every size.

The face is assembled from the several bitmaps Windows ships it in, one per writing system, which takes it from 219 characters to 469: Central European, Cyrillic, Greek, Turkish and Baltic text now draws from it. Right-to-left and stacking scripts are left to the scalable face, since they need reordering and mark placement the screens do not do.

A bitmap face cannot be resized, so it is used only where the game frame is drawn one screen pixel to one game pixel; at any other window size the scalable face is used whatever the setting says. Previously the bitmap face was stretched to fit at those sizes, which thinned and broke up the letters.

Recordings made before this release no longer load. The recording file holds the settings as a raw block, and adding a setting moves it.
