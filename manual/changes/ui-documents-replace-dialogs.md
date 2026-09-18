---
title: Draw every dialog as a UI document
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

Every screen outside a mission is now a UI document, read from the `ui` folder beside the
executable. The Win32 dialogs are gone.

A screen keeps the controls, the layout and the artwork of the dialog it replaces, and
opens with the same slide and the same sounds. Without the artwork it still opens, with
plain fills where the pictures would be.

Documents, styles and pictures are loaded by file name through the game's file system, so
a loose file or a mix entry replaces what ships. They are read again whenever a side mounts
its archives, so a side can carry its own.

Text fields take any character the dialog font can draw. A player whose keyboard writes
Cyrillic, Greek or accented Latin can type their name and their messages.

`BitmapSystemFont` picks the face those fields use. It is on by default and draws them in
the bitmap face the old dialogs used; clearing it uses the scalable face instead. The
bitmap face only answers where the game draws one screen pixel per game pixel, because it
cannot be resized.

Two things behave differently. The keyboard screen now drops its edits when you cancel,
where the dialog kept them if `KEYBOARD.INI` was missing. The frame-sync notice's Cancel
now leaves the match, where the dialog's button did nothing.

Recordings made before this release no longer load. The recording stores the settings as
one block, and adding a setting moves everything after it.
