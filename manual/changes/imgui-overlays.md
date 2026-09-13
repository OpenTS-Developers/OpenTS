---
title: Add the Dear ImGui developer overlays
category: internal
release: 0.2.0
targets:
  - type: command
    id: fixed:debug-benchmark-overlay
    effect: added
credit: [ZivDero]
---

A Debug build with the debug keys armed shows a frame benchmark window on F6, drawn by Dear ImGui over the game and its menus. It reports the logic frames and presents of the last second and the engine's frame benchmarks, and takes the mouse and keyboard only while the pointer is over it or one of its fields has focus.

The benchmarks it reads are the ones the monochrome Events page shows; the window resets them each second only while that display is off, so the two never take samples from each other. A Release build carries none of this.
