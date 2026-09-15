---
key: RadarEventSpeed
summary: Pixels of the radar pane a radar event's box loses from its radius each frame.
see_also: ["system:map-visibility", RadarEventMinRadius, RadarEventRotationSpeed]
when_omitted:
  kind: value
  value: "1"
---

A [radar event](/reference/enums/radar-event/) opens as a box whose corners stand as far from the flagged pixel as the furthest edge of the pane. The box then closes in on the cell it is flagging. Each frame the radius drops by this value, and the result is then held at no less than [`RadarEventMinRadius`](/keys/radareventminradius/). The sweep inward therefore runs for the gap between the opening radius and that floor, divided by this value. At the default of `1` that is one frame for every pixel of the gap. [The radar map](/systems/map-visibility/#the-radar-map) covers the pane the box is drawn over.

Landing on that floor is what lets the box settle. Only then does the event test whether it may stop turning, and only a settled event loads its timers and becomes eligible for removal.

:::caution[A non-positive value leaves every event on the radar for good]
At zero the radius never moves, and a negative value grows it. Unless [`RadarEventMinRadius`](/keys/radareventminradius/) already stands at or above the opening radius, the box never reaches the floor, so it never stops turning, never loads [`RadarEventDurations`](/keys/radareventdurations/) and is never removed. Every event raised goes on being drawn for the rest of the match, and for the three suppressible kinds goes on swallowing later events of its kind.
:::
