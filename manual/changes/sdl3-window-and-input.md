---
title: Run the game window and its input through SDL
category: internal
release: 0.2.0
targets: []
credit: [ZivDero]
---

The game window, its events, and keyboard and mouse input now go through SDL 3. The window opens at the same size and place, saved hotkeys keep their keys, and the pointer keeps its artwork and scale.

Tapping Alt or F10 no longer puts the window's system menu into keyboard mode, and Alt+Space no longer opens that menu.

A game played in a window can now be dragged onto another display or partly off the screen. It used to be held inside the primary display.
