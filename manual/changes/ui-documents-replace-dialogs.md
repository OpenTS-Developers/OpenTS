---
title: Draw every dialog as a UI document
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: added
- type: command
  id: fixed:main-menu-version
  effect: changed
- type: format
  id: keyboard-ini
  effect: changed
credit:
- ZivDero
---

Every screen the game shows outside a mission is now a UI document read from
the `ui` directory beside the executable, and the Win32 dialogs are gone: the
menus, the campaign and multiplayer choosers, the options family, the skirmish
setup and the map dialogs, the random map generator, the network lobbies, the
saved-game lists, the in-game options, the out-of-sync screen and the
frame-sync notice, the message boxes, and the notices shown while a game saves
or loads.

Each screen carries the controls of the dialog it replaces, at the size and in
the place that dialog put them, in the dialog art's own colors and pictures. A
screen opens as the dialogs did, sliding out from the middle behind the side
bars with the dialog sound, and a control sounds the same click. A player
without the art gets plain fills rather than a screen that will not open, and
with the pixel art filter the art keeps whole pixels at any window size.

Documents, styles and images load by bare file name through the game's file
system, so a loose file or a mix entry overrides what ships. The pictures and
the dialog font are read again whenever a side mounts its archives, so a side
can carry its own interface art. A screen whose document, style sheet or font
will not load names the file in the log and answers as though the player had
backed out of it.

Three screens say more than they did. The display options count the seconds
down before a mode is restored, where the dialog only said to wait. The
campaign chooser and the Internet game's speed bar name the setting they stand
at from the start, where the dialogs left a template caption standing until the
bar first moved. The frame-sync notice prints the lines naming the player being
waited for and what the wait offers, which only the dialog it replaced showed.

The keyboard screen stores a captured key as the same `KEYBOARD.INI` number the
Win32 dialog stored, without the extended-key flag that dialog's hotkey control
added to some keys.
