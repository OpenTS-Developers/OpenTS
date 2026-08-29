---
title: Keep the pointer within the system cursor size
category: fix
release: 0.1.0
targets:
- type: key
  id: CursorScale
  effect: changed
credit: [JusticarProgramming]
---

The pointer is drawn at the size its setting asks for times the size of its artwork. On a
display where the picture is enlarged a lot, that could come to a bitmap larger than the
cursor the operating system actually renders, and Windows then clips the cursor to its own
size. The clipped corner holds no artwork, so the pointer disappeared.

The pointer is now reduced to the largest whole size the system cursor can hold, so it always
shows. `CursorScale` still accepts its usual values; a size the display cannot render is
drawn as large as it can be.
