---
key: ScreenHeight
scope: client-settings-2
label: Height the display opens at
see_also: [ScreenWidth, WindowHeight, Fullscreen]
when_omitted:
  kind: computed
  note: Both dimensions become 640 by 480 when either is left out.
---

This is the earlier of the two reads of the assignment, made before the main window exists. [`ScreenWidth`](/keys/screenwidth/#scope-client-settings-2) covers what the pair does at that point: either dimension missing or written as `-1` replaces both, and the resulting size opens the window and sets the video mode.

The height is the full height of the game screen, and [the sidebar](/systems/sidebar/) is drawn down the whole of it. The depth and alpha buffers are allocated at a fixed 480 by 480 region as the display is brought up, whatever height was chosen. Loading a save or changing the display mode later reallocates them at the tactical view's real size.

Starting the game with the internet debug view showing sets the pair to 640 by 400 before the display is opened, whatever the file said.
