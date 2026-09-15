---
key: IntegerScaling
summary: Whether the picture is only ever enlarged by whole multiples.
when_omitted:
  kind: value
  value: "no"
---

The picture normally grows to fill as much of the window as its shape allows, whatever fraction of the window that turns out to be. Switch this on and the enlargement is rounded down to a whole number instead, so every drawn pixel becomes an exact square block and none of them is a different size from its neighbors.

The cost is unused space, and it can be a lot of it. A 640 by 400 picture in a 1280 by 800 window doubles exactly and fills it either way. The same picture in a 1400 by 900 window doubles rather than growing 2.19 times, leaving a margin all around. In a 1200 by 900 window it cannot manage a second whole multiple at all and is drawn at its own size in the middle of the window.

This applies only while the picture is being enlarged. A window smaller than the rendering resolution shrinks the picture as usual, since rounding down there would leave nothing to show.

[`ScaleMode`](/keys/scalemode/) chooses how the picture is filtered. Its `PixelArt` setting already keeps pixels sharp at fractional sizes, so switching this on as well is only needed when exactly equal pixels matter more than window space.
