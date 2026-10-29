---
title: Enlarge the pointer with the Windows display scale
category: fix
release: 0.2.0
targets:
- type: key
  id: CursorScale
  effect: changed
credit:
- fjlaubscher
---

With `CursorScale=0`, the pointer now grows to the Windows display scale when that scale is larger than the picture's enlargement, so a 3840 by 2160 picture on a display set to 200% gets a double-size pointer. It used to stay at the artwork's size whenever the picture was not enlarged. A positive or negative `CursorScale` is unchanged, and `SystemCursor=yes` still uses the Windows pointer's own size.
