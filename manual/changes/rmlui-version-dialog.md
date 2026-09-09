---
title: Show the version dialog as an RmlUi document
category: feature
release: 0.2.0
targets:
- type: key
  id: LegacyDialogs
  effect: added
- type: system
  id: ui-files
  effect: added
- type: command
  id: fixed:main-menu-version
  effect: changed
credit:
- ZivDero
---

The version dialog is now an RmlUi document drawn over the title screen, opened from the main menu entry and Ctrl+V as before and closed by OK, Enter or Escape. `sun.ini` gains `LegacyDialogs` under `[Options]`; set to `yes`, it keeps the Win32 dialog for this and every later screen that gains a document.

The documents, style sheets and font ship in a `ui` directory beside the executable and load by bare file name through the game's file system, so a loose file or a mix entry can override them. A document, style sheet or font that fails to load falls back to the Win32 dialog.
