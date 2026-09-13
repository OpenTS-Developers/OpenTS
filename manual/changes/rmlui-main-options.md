---
title: Show the options menu as an RmlUi document
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The options menu the main menu opens is an RmlUi document with the same five buttons in the same place, Game Settings, Display, Sound, Keyboard and Main Menu, and the Sound button still goes dead without an audio device. The settings are still written when the player leaves. `LegacyDialogs=yes` keeps the Win32 dialog.
