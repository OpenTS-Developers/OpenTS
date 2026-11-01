---
key: ScreenWidth
scope: client-settings-2
label: Width the display opens at
see_also: [ScreenHeight, WindowWidth, Fullscreen]
when_omitted:
  kind: computed
  note: A dimension left out keeps the size given on the command line, if any. If either dimension is still unset, both become the largest whole fraction of the primary display that keeps the picture within 960 by 540 and at least 640 by 400, or the display's own size when none does.
---

The game opens its screen at `ScreenWidth` by [`ScreenHeight`](/keys/screenheight/#scope-client-settings-2) pixels. This read happens at startup, before the game window exists. After that, only the display options screen changes the size.

Writing `-1` for either dimension makes both the default size for the display, even when the launch option gives a size. The default is a whole fraction of the display, so the picture grows by an exact factor.

A size written in `sun.ini` wins over the [resolution launch option](/using/command-line/#resolution), which fills in only a dimension the file leaves out.

If the renderer cannot start at the chosen size, the game shows a video error and exits.

[The sidebar](/systems/sidebar/) takes 168 pixels of the width, and the tactical view gets the rest.
