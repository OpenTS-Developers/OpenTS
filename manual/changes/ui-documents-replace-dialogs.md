---
title: Draw every dialog as a UI document
category: feature
release: 0.2.0
targets:
- type: key
  id: LegacyDialogs
  effect: removed
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

Every screen the game shows outside a mission is now a UI document from the `ui` directory: the main menu and the menus under it, the campaign and multiplayer choosers, the options menu with the sound, display, game control and keyboard screens, the skirmish setup and the map dialog, the random map generator, the network lobbies, the load, save and delete lists, the in-game options and the abort question, the out-of-sync screen and the frame-sync notice, the message boxes, and the notices shown while a game saves or loads.

Each carries the controls of the dialog it replaces, at the size and in the place that dialog put them. The Win32 dialogs themselves are gone, and with them the `LegacyDialogs` setting that chose between the two. A screen whose document, style sheet or font will not load names the file in the log and answers as though the player had backed out of it.

The frame-sync notice now prints the lines that name the player being waited for and what the wait offers, which only the dialog it replaced used to show.
