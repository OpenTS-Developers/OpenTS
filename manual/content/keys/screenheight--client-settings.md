---
key: ScreenHeight
scope: client-settings
label: Stored height
see_also: [ScreenWidth, Fullscreen]
when_omitted:
  kind: value
  value: "480"
  note: The earlier read has already replaced any pair with a missing half by 640 by 480 and opened the display at that size, so this read falls back to the height the screen runs at.
---

This is the later of the two reads of the assignment, made with the rest of the client settings once the display is already open. [`ScreenWidth`](/keys/screenwidth/#scope-client-settings) covers what the pair settles at that point: the mode the display options screen starts from, and the size the game falls back to when a mode tried there is declined. Writing `-1` rather than leaving the assignment out is not the same thing, for the reason given there.
