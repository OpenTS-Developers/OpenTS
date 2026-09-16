---
title: Add the developer overlays and the UI test document
category: internal
release: 0.2.0
targets:
  - type: command
    id: fixed:debug-benchmark-overlay
    effect: added
  - type: command
    id: fixed:debug-ui-test-document
    effect: added
credit: [ZivDero]
---

A Debug build with the debug keys armed shows a frame benchmark window on F6,
drawn by Dear ImGui over the game and its menus. It reports the logic frames
and presents of the last second and the engine's frame benchmarks, and takes
the mouse and keyboard only while the pointer is over it or one of its fields
has focus.

The benchmarks it reads are the ones the monochrome Events page shows; the
window resets them each second only while that display is off, so the two never
take samples from each other.

The same build shows a UI test document on F9, which is a panel with a button
that closes it. It exercises the document path the screens use, and each load,
show and hide writes the renderer's texture and buffer counts to the log, so a
leak across repeated toggles shows there.

A Release build carries none of this.
