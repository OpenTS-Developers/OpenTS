---
key: ScaleMode
summary: How the picture is filtered when the window is larger than the resolution the game renders at.
when_omitted:
  kind: value
  value: PixelArt
---

The game renders at [`ScreenWidth`](/keys/screenwidth/) by [`ScreenHeight`](/keys/screenheight/) and that picture is scaled into the window. This setting chooses how it is filtered on the way, and accepts three names, in any capitalization:

| Value | Result |
| --- | --- |
| `PixelArt` | Sharp pixels whenever the window is larger than the picture |
| `Nearest` | Plain pixel doubling, at the cost of uneven pixel sizes |
| `Linear` | Smooth, at the cost of blurring the artwork |

`PixelArt` is a compromise between the other two. A 640 by 400 picture in a 1280 by 800 space is an exact multiple, and there it does what `Nearest` does, because whole pixels already land on whole pixels. At any other size above the picture it magnifies to the next whole multiple with hard edges, then shrinks that smoothly into the window. The artwork stays sharp without the uneven pixel sizes that `Nearest` alone produces at fractional scales.

The size only matters to `PixelArt`. Its whole-multiple work needs a window larger than the picture in both dimensions, and anything else falls back to the smooth filtering that `Linear` uses at every size. `Nearest` samples the same way whether the picture is being enlarged or reduced.

A name the game does not recognize leaves the filter as it was, which is always `PixelArt` when the game reads the file.

Set [`IntegerScaling`](/keys/integerscaling/) to confine the picture to whole multiples instead of filling the window.
