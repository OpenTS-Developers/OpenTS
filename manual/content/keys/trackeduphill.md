---
key: TrackedUphill
summary: Speed multiplier for a tracked vehicle stepping to a higher cell.
see_also: [TrackedDownhill, WheeledUphill, WheeledDownhill, SpeedType]
when_omitted:
  kind: value
  value: "1"
---

Each cell of a vehicle's journey is costed as a fraction of full speed, drawn from the movement table its [`SpeedType`](/keys/speedtype/) selects. That fraction is multiplied by this value when the ground under the destination cell stands higher than the ground under the vehicle. A step of two or more terrain levels is costed from the Road row of that table rather than from the destination's own land type. Ground heights are what is compared, so a ramp counts as a climb exactly as a cliff edge does. A step between cells of equal height takes neither this multiplier nor [`TrackedDownhill`](/keys/trackeddownhill/).

The multiplier reaches vehicles only, and among them only those with `SpeedType=Track`; every other SpeedType on a vehicle takes [`WheeledUphill`](/keys/wheeleduphill/) instead, whatever its name suggests. It is also the ordinary ground drive's rule alone, so a vehicle given any other [`Locomotor`](/keys/locomotor/), such as hover or tunnel, climbs at an unmodified speed. Only the speed of the step changes: the route search does not read this value, so a slower climb does not push a vehicle to route around the hill.

:::caution[The multiplier is applied after the terrain cost is capped]
The terrain cost is clamped to full speed first, and this value is applied afterwards. A figure above `1` therefore moves a tracked vehicle uphill faster than it moves on the flat, and nothing clamps the product from above. The stock `.5` halves the speed the terrain would otherwise allow. A product of exactly zero is then replaced with half of full speed, so `TrackedUphill=0` does not forbid a climb.
:::
