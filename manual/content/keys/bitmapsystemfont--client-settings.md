---
key: BitmapSystemFont
scope: client-settings
label: Bitmap dialog text
when_omitted:
  kind: value
  value: "yes"
---

`BitmapSystemFont=yes` draws the lists, text boxes, tooltips, hotkey fields and group headings of the game's screens in the fixed-size bitmap face Windows ships, which is the face the original dialogs used. `BitmapSystemFont=no` draws them in the scalable face of the same design. Captions and button text come from the game's own art and look the same either way.

A bitmap face is cut at one size and cannot be resized without losing its shape, so it is used only while each game pixel is drawn as one screen pixel, as in a window the size of the game's resolution. While the game is drawn larger or smaller, the scalable face is used whatever this key says.

Windows ships the bitmap face as one file per writing system and the game reads them all, so Central European, Cyrillic, Greek, Turkish and Baltic text draws from it as well as Western. A character none of these files holds, such as an Arabic or Hebrew letter, draws as a question mark in the bitmap face; with `BitmapSystemFont=no` it draws from the scalable face, if that face has it.

The key is read as the game starts and no screen offers it, so a change takes effect the next time the game runs. On a machine without the bitmap files installed, the scalable face is used.
