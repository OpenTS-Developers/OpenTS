---
title: Drop the keyboard dialog's edits on Cancel
category: fix
release: 0.2.0
targets:
- type: format
  id: keyboard-ini
  effect: changed
credit:
- ZivDero
---

The keyboard dialog edits a copy of the hotkey table and hands it to the game only when the player accepts. Cancel used to reload `KEYBOARD.INI` over the live table, which kept the edits whenever the file was missing or unreadable; it now drops them in every case.
