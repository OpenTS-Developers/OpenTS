---
title: Draw UI documents with the dialog kit
category: internal
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

A shared style sheet and template, `kit.rcss` and `dialog.rml`, give every UI document the look of the game's own dialogs: the wallpaper, the side bars and the glow around a screen, and the buttons and other controls in the colors and sizes the dialogs draw them with. The pictures are the game's own interface art; a control keeps a plain fill where one is missing.

A screen opens as the original dialogs did, sliding out from the middle behind the side bars with the dialog sound, and a document can turn that off for itself. Pressing a control sounds the same click, which the rules name, and a disabled control stays silent.

Every migrated screen is rebuilt on the kit at the size and spacing its original dialog had, so the options menu, the sound and display options, the game controls, the keyboard screen, the message boxes, the mode confirmation, the wait notice and the version dialog all look like the dialogs they replace rather than the plain layout they had. The keyboard screen picks its category from a drop-down list again, and the sound sliders show their volume beside them again.

With the pixel art filter, the dialog art keeps whole pixels at any window size the way the game frame does, instead of blurring at scales that are not a whole number.
