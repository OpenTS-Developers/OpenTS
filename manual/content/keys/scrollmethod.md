---
key: ScrollMethod
summary: Which of three coast-scroll behaviors the held right mouse button drives.
see_also: [ScrollRate, AutoScroll]
when_omitted:
  kind: value
  value: "0"
---

The figure applies while the right mouse button is held down over the tactical map, and the distance scrolled follows from how far the pointer has moved away from the point where the button went down.

| Figure | Behavior |
| --- | --- |
| `0` | The map scrolls by that offset divided by the scroll rate every thirtieth of a second, and the pointer is left where it is. |
| `1` | The map scrolls twelve times as far and the pointer is put back on the press point, so the ground appears to slide under a pointer that never moves. Each stretch of hand movement is consumed once, so the map keeps pace with the hand rather than the clock. |
| `2` | The same, with the offset negated, so the ground follows the pointer instead of running away from it. |

The in-game game controls dialog offers the first two as a single check box and writes the choice back to `sun.ini`. `2` is reachable only by writing it into the file.

:::caution[A figure outside 0 through 2 disables coast scrolling]
The three behaviors are the only cases handled, and the horizontal and vertical distances are worked out inside them. A figure of `3` or more, or a negative one, falls through all three and leaves both distances at zero, so holding the right button drags the view nowhere.
:::
