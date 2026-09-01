---
key: ScrollRate
summary: How fast the tactical map is allowed to scroll, as a position from 0 to 6 where a lower figure scrolls faster.
see_also: [ScrollMethod, AutoScroll, ScrollMultiplier]
when_omitted:
  kind: value
  value: "3"
---

Resting the pointer against the edge of the screen scrolls the map, and the map gathers speed the longer the pointer is held there. The distance covered each thirtieth of a second comes from a fixed table of nine steps, spread across however many frames the display draws in that time, and this figure fixes how far along that table the map may climb: `0` allows the second-longest of the nine steps and `6` a short one; the longest step in the table is out of reach for every figure the file can sensibly carry. Holding the right mouse button down while the map is scrolling from the edge shortens the step instead of lengthening it, and pins it to the slower half of the table. [`ScrollMultiplier`](/keys/scrollmultiplier/) then scales whatever step was chosen.

The figure also divides the coast-scroll distance described by [`ScrollMethod`](/keys/scrollmethod/): the further the pointer is dragged from the point where the right button went down, the further the map moves, and this figure plus one is what that offset is divided by.

The in-game game controls dialog offers seven positions and writes the choice back to `sun.ini`. Its slider runs the other way around, so dragging it toward the fast end stores a smaller figure.

:::danger[A figure of eight or more reads past the scroll table]
Nothing narrows the figure on the way in. A figure of `8` or more indexes past the end of the nine-step edge-scroll table. `-1` is milder but still wrong: it makes the coast-scroll divisor zero, and the distance the map is then scrolled by is whatever converting an infinite quantity to a whole number leaves behind.
:::
