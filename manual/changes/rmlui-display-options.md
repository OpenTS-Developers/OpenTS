---
title: Show the display options as RmlUi documents
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The display options open as an RmlUi document from the options menu, with the same resolution list and movie switch, and the confirmation shown after a mode change is a document too. Accepting still stores the switch at once and tries a new mode first, and a mode the player does not keep within ten seconds is still restored; the document counts the seconds down where the Win32 dialog only said to wait. `LegacyDialogs=yes` keeps both Win32 dialogs.
