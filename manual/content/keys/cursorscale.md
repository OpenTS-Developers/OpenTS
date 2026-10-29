---
key: CursorScale
summary: How many times larger than its artwork the mouse pointer is drawn.
when_omitted:
  kind: value
  value: "0"
  note: Zero matches the pointer to how much the picture itself is enlarged on screen, and on Windows also to the display scale when that is larger.
---

A positive value sets the pointer's size directly: `2` draws it at twice the size of its artwork. A value above `8` acts as `8`. A negative value draws the pointer at the size of its artwork however large the picture is displayed.

At `0`, the game takes the smaller of the picture's horizontal and vertical enlargement and rounds it to the nearest whole number, so a picture displayed at about 2.2 times its rendered size gets a double-size pointer. On Windows, a display scale larger than that enlargement is used instead: a 3840 by 2160 picture on a display set to 200% gets a double-size pointer. The pointer is never drawn smaller than its artwork, or more than eight times larger. When the window is resized, a pointer set to `0` is rebuilt at its new size.

The same size applies to the arrow shown over the game's menus and dialogs, unless [`SystemCursor`](/keys/systemcursor/) replaces that arrow with the Windows pointer, which keeps the size set in Windows.

Windows draws the pointer over the picture, so it does not grow when the picture is enlarged. This keeps it moving smoothly while the game is busy.
