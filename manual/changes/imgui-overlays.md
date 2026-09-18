---
title: Add the developer overlays
category: internal
release: 0.2.0
targets:
  - type: command
    id: fixed:debug-benchmark-overlay
    effect: added
credit: [ZivDero]
---

A Debug build with the debug keys armed shows a frame benchmark window on F6, drawn over
the game and its menus. It reports the logic frames and presents of the last second, and
the engine's frame benchmarks.

The window takes the mouse and the keyboard only while the pointer is over it or one of its
fields has focus. Everything else reaches the game.

A Release build carries none of this.
