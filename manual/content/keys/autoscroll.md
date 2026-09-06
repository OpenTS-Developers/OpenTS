---
key: AutoScroll
summary: Scrolls the tactical map while the pointer rests against the edge of the screen.
see_also: [ScrollRate, ScrollMethod, ScrollMultiplier]
when_omitted:
  kind: value
  value: "yes"
---

`AutoScroll=no` stops that scrolling and leaves the pointer as it is instead of turning it into the directional scroll arrow. The keyboard scroll keys, the coast scroll [`ScrollMethod`](/keys/scrollmethod/) describes, and clicking the radar all still move the view.

[`ScrollMultiplier`](/keys/scrollmultiplier/) scales edge scroll steps alone, so it has nothing to act on once this is off. [`ScrollRate`](/keys/scrollrate/) also divides the coast scroll distance and keeps working either way.

The in-game game controls dialog carries the same switch and writes the choice back to `sun.ini`. Changing it there takes effect at once rather than at the next scenario.
