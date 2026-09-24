---
title: Draw the dialogs as UI documents
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: added
- type: key
  id: BitmapSystemFont
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

The game's dialogs, including the classic main menu and the options menus, are now screens drawn from UI documents in the `ui` directory beside the executable, and dialog templates in `Language.dll` are no longer used. The score screens, the graphical main menu, the mission briefing and the sidebar are drawn as before.

A screen keeps the controls, layout, artwork and sounds of the dialog it replaced, and opens with the same slide. Where a picture is missing, the control draws a plain fill and the screen still opens.

When a screen is driven from the keyboard, the control that holds the focus is marked in white until the next mouse press. The Win32 dialogs drew no focus mark.

Documents, style sheets and pictures are found by bare file name through the game's file system. A file of the same name in a folder the game searches before `ui` replaces the shipped one.

Text fields take any character their face can draw. A player whose keyboard writes Cyrillic, Greek or accented Latin can type their name and their messages.

`BitmapSystemFont` in `SUN.INI` chooses the face of the screens' lists, text boxes, tooltips, hotkey fields and group headings: `yes` uses the bitmap face the old dialogs used, and `no` the scalable face of the same design. The bitmap face cannot be resized, so it is used only where the game draws one screen pixel per game pixel.

Canceling the keyboard screen now drops the edits made in it. The dialog kept them whenever `KEYBOARD.INI` was missing.

Recordings made before this release no longer load. A recording stores the game's settings as one block, and the new `BitmapSystemFont` setting moves everything after it.
