---
key: ScreenWidth
scope: client-settings
label: Stored width
see_also: [ScreenHeight, Fullscreen]
when_omitted:
  kind: computed
  note: The startup read has already opened the screen, so this read keeps the width the screen runs at.
---

This second read happens with the rest of the client settings, after the screen is already open, and it does not resize the screen. The width it stores is the mode the display options screen starts from, and the width the game returns to when a new mode is declined there.

Choosing a new mode on the display options screen asks for confirmation and then switches to that mode on trial. The game stores the new width only if the player keeps the mode. Declining it, or letting the confirmation time out, switches back to the stored size.

Leaving the options menu, or accepting the game controls dialog, saves the stored width to `sun.ini` with every other client setting. The file therefore has a `ScreenWidth` once either has been used.

:::caution[Do not write -1]
The startup read treats `-1` as a missing width and opens the screen at the default size for the display, but this read stores the `-1` as written. The display options screen then starts with no mode selected. Declining a new mode there leaves the declined mode on screen, because the game cannot switch back to a width of `-1`. The `-1` is also saved back to the file.
:::
