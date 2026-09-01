---
key: WindowHeight
summary: The height of the drawable area of the game window, in pixels.
when_omitted:
  kind: computed
  note: The window opens at the height the game renders at, ScreenHeight, and afterwards follows it through a resolution change.
---

This is the size of the window's drawable area rather than its outer size, so the border and title bar are added on top of it. The window opens centered on the screen.

[`WindowWidth`](/keys/windowwidth/) owns how the pair behaves, when the window stops following the rendering resolution, and why a window whose shape does not match the picture's shows bars rather than stretching it.

The value applies only while the game is windowed. A full-screen game covers the desktop and ignores it.
