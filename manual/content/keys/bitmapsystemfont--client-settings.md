---
key: BitmapSystemFont
scope: client-settings
label: Bitmap dialog text
when_omitted:
  kind: value
  value: "yes"
---

`BitmapSystemFont=yes` draws the lists, text boxes, control tooltips, hotkey fields and group headings of the game's screens in the fixed-size bitmap face that Windows ships, the face the original dialogs used. `BitmapSystemFont=no` draws them in the scalable face of the same design. Captions and button text use the game's own font art and look the same at either setting.

The bitmap face is used only while each game pixel is drawn as one screen pixel, as in a window the size of the game's resolution. While the game is drawn larger or smaller, the scalable face is used at either setting, because a bitmap face loses its shape when it is resized.

Windows ships the bitmap face as one file per writing system, and the game loads them all. Western, Central European, Cyrillic, Greek, Turkish and Baltic text therefore draws in the bitmap face. A character that none of these files holds, such as an Arabic or Hebrew letter, draws as a question mark in the bitmap face. With `BitmapSystemFont=no` it draws from the scalable face, if that face has it.

On a machine without the bitmap face files installed, the scalable face is used.

The game reads the key at startup, and no screen offers it, so a change takes effect the next time the game runs.
