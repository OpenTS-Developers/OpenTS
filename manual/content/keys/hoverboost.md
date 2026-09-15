---
key: HoverBoost
summary: Multiplier on the throttle ceiling of a hovering unit whose path continues straight ahead.
see_also: [HoverAcceleration, HoverBrake]
when_omitted:
  kind: value
  value: "1.3"
---

A hover drive works a throttle up and down between nothing and full, and the ground the unit covers each frame is its own travel speed times that throttle. This value multiplies the ceiling the throttle is allowed to climb toward. It applies only while the next two steps of the unit's path face the same way, so a unit running down a straight line may open up further than one weaving between cells. It is a multiplier on that ceiling rather than a speed of its own; the unit's [`Speed=`](/keys/speed/) still fixes what full throttle is worth. The stock `150%` is read as `1.5`.

:::caution[The boosted ceiling is clamped to full throttle]
The multiplied ceiling is capped at full throttle before the drive compares it against the throttle the unit is holding. A value above `1` therefore cannot push a hovering unit past its own top speed; it can only bring a ceiling the drive has already lowered back up toward full. A value below `1` holds a straight run under full speed instead.
:::
