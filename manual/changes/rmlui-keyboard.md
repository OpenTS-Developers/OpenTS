---
title: Show the keyboard dialog as an RmlUi document
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
- type: format
  id: keyboard-ini
  effect: changed
credit:
- ZivDero
---

The keyboard dialog opens as an RmlUi document from the options menu and from the in-game game controls, with the categories and commands as lists, the description and current shortcut of the selected command, a capture box that takes the next key pressed with its modifiers, Assign, Reset All, OK and Cancel. A captured key is stored as the same `KEYBOARD.INI` number the Win32 dialog stored, without the extended-key flag that dialog's hotkey control added to some keys. `LegacyDialogs=yes` keeps the Win32 dialog.
