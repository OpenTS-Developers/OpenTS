---
title: Show the pointer when a dialog takes the mouse
category: fix
release: 0.2.0
targets:
- type: key
  id: CursorScale
  effect: changed
credit: [JusticarProgramming]
---

When the game gave the mouse back to the operating system so a dialog could use it, the
pointer was hidden and stayed hidden until it next moved. The release path cleared the cursor
but nothing put it back, so a dialog that opened under the pointer came up over a blank cursor.

The release path now restores the arrow as it gives up the mouse, so the pointer is visible
the moment a dialog appears.
