---
key: RadarEventMinRadius
summary: Pixels of the radar pane a shrinking radar event box stops at.
see_also: ["system:map-visibility", RadarEventSpeed]
when_omitted:
  kind: value
  value: "5"
---

A [radar event](/reference/enums/radar-event/) gives up [`RadarEventSpeed`](/keys/radareventspeed/) pixels of radius a frame until it reaches this floor. That is the size the settled marker keeps for the rest of its life. The radius runs from the flagged pixel out to a corner of the box, which therefore spans twice it at its widest.

The clamp is also what makes settling possible. The event only tests whether it may stop turning on a frame when its radius sits within a hundredth of a pixel of this value. The clamp lands it there exactly. A floor at or above the box's opening radius is met on the first frame, so the box begins testing to settle at once. It comes to rest holding a marker whose corner-to-corner span is at least the pane's longer side.

`RadarEventMinRadius=0` collapses all four corners onto the flagged pixel. The resulting edge length of zero is then used as a divisor with no guard when the rate of the box's color gradient is computed.
