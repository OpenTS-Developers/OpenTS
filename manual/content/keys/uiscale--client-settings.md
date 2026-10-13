---
key: UIScale
scope: client-settings
label: Interface size
see_also: [ScreenWidth, ScreenHeight, CursorScale]
when_omitted:
  kind: value
  value: "0"
  note: Zero picks the size from the screen height.
---

`UIScale` in `[Video]` of `sun.ini` sets how many times larger than its artwork the interface is drawn: the [sidebar](/systems/sidebar/) with its radar and cameos, the strip across the top with the Options tab and mission timer, and the game's screens and dialogs. `1` to `4` choose the size; a larger value acts as `4`. `0` or a negative value picks it from the height set by [`ScreenHeight`](/keys/screenheight/#scope-client-settings-2): `1` below 2160 rows, `3` from 2160 rows, and `4` from 4320 rows.

At 3840 by 2160 the automatic size is `3`. The sidebar is then 504 pixels wide, the top strip 48 pixels high, and the [battlefield](/systems/battlefield-zoom/) fills the remaining 3336 by 2112 pixels at one screen pixel per map pixel. Battlefield zoom still magnifies that area.

The size is lowered one step at a time until the interface keeps at least 640 by 540 of its own pixels and the battlefield beside it keeps at least 472 by 524 pixels. A chosen `3` at 1920 by 1080 therefore draws the interface at `2`.

The Display screen of the main menu's Options sets the size: each press of the Interface size button steps it from Automatic through 1x to 4x and back, and OK keeps the choice. A new size takes effect when the next mission or skirmish starts or a saved game is loaded; the in-game Options menu does not offer it.

Chat and game messages are drawn at the same size as the interface. Tooltips over the battlefield are drawn at artwork size. The main menu and its screens keep their artwork size at every setting.
