---
key: ScreenWidth
scope: client-settings-2
label: Width the display opens at
see_also: [ScreenHeight, WindowWidth, Fullscreen]
when_omitted:
  kind: computed
  note: Both dimensions become 640 by 480 when either is left out.
---

This is the earlier of the two reads of the assignment, made before the main window exists. Either dimension left out or written as `-1` replaces both with 640 by 480, so a width alone is never enough to size the screen. The game screen is then created at the resulting size, and the game reports a video error and stops if the renderer cannot start.

The size also fixes the layout the game is drawn into: [the sidebar](/systems/sidebar/) takes a fixed 168 pixels of the width, and the tactical view is given what is left.
