---
key: CursorScale
summary: How many times larger than its artwork the mouse pointer is drawn.
when_omitted:
  kind: value
  value: "0"
  note: Zero matches the pointer to how much the picture itself is enlarged on screen.
---

The pointer is a real system cursor built from the game's own artwork, so the system keeps drawing and moving it even while the game is busy. It is drawn over the picture rather than inside it, which means it does not grow with the picture on its own and needs its own size.

Left at `0` it follows the picture: the enlargement is rounded to the nearest whole number and used as the pointer's size, so a picture displayed at roughly double size gets a double-size pointer. A value above zero sets the size directly, and eight is the largest accepted. A value below zero draws the pointer at the size the artwork was drawn at, however large the picture is displayed.

The pointer is rebuilt whenever the size it should be drawn at changes, so resizing the window resizes the pointer with it.

However large the size asks to be, the pointer is held to the size the operating system can draw a cursor at. A display that cannot show the requested size draws the pointer as large as it can rather than clipping it to an invisible corner.
