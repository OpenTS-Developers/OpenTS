---
key: ScrollMultiplier
summary: Factor applied to the distance each edge scroll step moves the tactical view.
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[AudioVisual]
ScrollMultiplier=1.5
```

Holding the cursor against an edge of the tactical view scrolls the map by a step whose length comes from a fixed nine-entry table running from 16 up to 448 pixels. A step is what the view covers in a thirtieth of a second. Which entry is used depends on how long the cursor has been held there, bounded by the player's own scroll speed option; the entry is then multiplied by this value and truncated to whole pixels. Doubling the value doubles every step in that ramp without changing how quickly the ramp is climbed.

Only edge scrolling is scaled. Dragging the view with the right mouse button works from its own rate and never consults this value, so raising it makes the two travel at noticeably different speeds.

:::caution[Truncation can stop the slowest steps entirely]
Each step is cut to a whole number of pixels after the multiplication. At `0.05` the shortest entry in the table comes out at zero and the view stands still until the ramp has climbed far enough to produce a step of one pixel or more. A value of `0` holds the view still at every point of the ramp.
:::
