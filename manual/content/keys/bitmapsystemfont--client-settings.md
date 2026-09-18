---
key: BitmapSystemFont
scope: client-settings
label: Bitmap dialog text
when_omitted:
  kind: value
  value: "yes"
---

The dialogs draw their lists, text boxes, tooltips and hotkey fields in the face Windows keeps as a set of fixed-size bitmaps, which is the face the game's dialogs were drawn with. Clearing this draws them in the scalable face of the same design instead. The captions and the button text are unaffected either way: those come from the game's own art rather than from a system face.

A bitmap is cut at one size and cannot be resized without losing its shape, so the bitmap face is used only where the game frame is drawn one screen pixel to one game pixel. At any other window size the scalable face is used whatever this says, which is why the text may change appearance when the resolution does.

The face is put together from the several bitmaps Windows ships it in, one per writing system, so it is not limited to Western letters: Central European, Cyrillic, Greek, Turkish and Baltic text draws from it as well. Right-to-left and stacking scripts are left to the scalable face.

The setting is read as the game starts and no dialog offers it, so a change takes effect the next time the game runs. A machine without the bitmaps installed falls back to the scalable face on its own.
