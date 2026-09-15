---
key: HoverAcceleration
summary: Time in minutes a hovering unit takes to work its throttle up to full.
see_also: [HoverBrake, HoverBoost]
when_omitted:
  kind: value
  value: ".03"
---

The figure is a length of time, not a rate, so raising it makes a hover unit slower off the mark. Each frame the drive raises the throttle by `1 ÷ (HoverAcceleration × 900)` of full and stops at the ceiling it is holding, which [`HoverBoost`](/keys/hoverboost/) scales. At 900 frames to the minute that makes the value the length of a standing start, so the stock `.02` is 18 frames, a little over a second.

Only the climb is measured here. Closing the throttle again runs on [`HoverBrake`](/keys/hoverbrake/).
