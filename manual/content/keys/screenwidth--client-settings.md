---
key: ScreenWidth
scope: client-settings
label: Stored width
see_also: [ScreenHeight, Fullscreen]
when_omitted:
  kind: value
  value: "640"
  note: The earlier read has already replaced any pair with a missing half by 640 by 480 and opened the display at that size, so this read falls back to the width the screen runs at.
---

This is the later of the two reads of the assignment, made with the rest of the client settings once the display is already open. It cannot change the size of the screen the game is drawing on; what it settles is the size the display options screen starts from.

Accepting the display options screen stores the mode picked there. Leaving the options screen behind it writes the stored width back to `sun.ini`, so the file normally has one once that screen has been used.

:::caution[Writing the fallback figure is not the same as leaving the assignment out]
A `-1` written into the file is read back here, after the screen has already been sized around it, and the stored width stays at `-1`. No mode in the display options list matches it, and when a mode tried there is declined the game asks the display to switch back to a width of `-1` pixels.
:::
