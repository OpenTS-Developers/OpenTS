---
key: ScreenHeight
scope: client-settings-2
label: Height the display opens at
see_also: [ScreenWidth, WindowHeight, Fullscreen]
when_omitted:
  kind: computed
  note: A height left out keeps the launch option's height, if any. If either dimension is still unset, both become the largest whole fraction of the primary display that keeps the picture within 960 by 540 and at least 640 by 400, or the display's own size when none does.
---

The game reads this height with [`ScreenWidth`](/keys/screenwidth/#scope-client-settings-2) before it opens its window, and renders at the resulting size. That page covers how the pair combines with the launch option, the default size for the display, and what happens when the renderer cannot start.

The height is the full height of the game screen. [The sidebar](/systems/sidebar/) runs down the whole of it, and the tactical view takes the height left below a 16-pixel strip across the top.
