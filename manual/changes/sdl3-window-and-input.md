---
title: Run the game window and its input through SDL
category: internal
release: 0.2.0
targets: []
credit: [ZivDero]
---

The game window, its events, and keyboard and mouse input now go through SDL 3. The window opens at the same size and place, saved hotkeys keep their keys, and the pointer keeps its artwork and scale.

Tapping Alt or F10 no longer puts the window's system menu into keyboard mode, and Alt+Space no longer opens that menu.

A game played in a window can now be dragged onto another display or partly off the screen. It used to be held inside the primary display. The rules chooser, which appears when more than one `RULE*.INI` file is found, can be dragged the same way, and F1 or a right-click on it no longer tries to open the old help file.

The game no longer reserves Ctrl+Alt+Shift+M for itself, so other programs can use that shortcut. The messages shown when the renderer or the game cannot start now use SDL's message box, with an error icon.

In-game chat, the high-score name and the older dialogs' text boxes now take the characters the keyboard layout types, including accents typed with a dead key and text from an input method. The keyboard options screen names a key by the character the layout prints on it, or by an English name such as `Home` for a key that prints none; it used to show the names Windows gives in its own language.
