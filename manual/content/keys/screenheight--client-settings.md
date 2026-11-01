---
key: ScreenHeight
scope: client-settings
label: Stored height
see_also: [ScreenWidth, Fullscreen]
when_omitted:
  kind: computed
  note: The startup read has already opened the screen, so this read keeps the height the screen runs at.
---

The game reads this height a second time with the rest of its settings, after the display is already open. This read does not resize the screen. It sets the height the display options screen starts from, and the height the game returns to when a mode tried there is declined.

[`ScreenWidth`](/keys/screenwidth/#scope-client-settings) covers the stored pair in full, including why writing `-1` is not the same as leaving the assignment out.
