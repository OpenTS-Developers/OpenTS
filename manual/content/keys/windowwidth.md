---
key: WindowWidth
summary: The width of the drawable area of the game window, in pixels.
when_omitted:
  kind: value
  value: "-1"
  note: The window opens at the width the game renders at, ScreenWidth.
---

This is the size of the window's drawable area rather than its outer size, so the border and title bar are added on top of it. The window opens centered on the screen.

Setting it takes the window out of the game's hands: a window sized by the player keeps its size when the rendering resolution changes, and the picture is scaled to fit it instead. Leave both this and [`WindowHeight`](/keys/windowheight/) unset (or at zero or below) and the window follows [`ScreenWidth`](/keys/screenwidth/) and [`ScreenHeight`](/keys/screenheight/) instead, resizing about its own middle each time the resolution changes.

The value applies only while the game is windowed. A full-screen game covers the desktop and ignores it. Resizing the window by dragging its edge does not write a new value back; the size stored here is the one the window opens at.

The picture keeps its shape whatever the window's proportions are, so a window that does not match adds bars on two sides rather than stretching. [`ScaleMode`](/keys/scalemode/) chooses how the picture is filtered on the way up.
