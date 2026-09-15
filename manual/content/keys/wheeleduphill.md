---
key: WheeledUphill
summary: Speed multiplier for a vehicle that is not tracked stepping to a higher cell.
see_also: [WheeledDownhill, TrackedUphill, TrackedDownhill, SpeedType]
when_omitted:
  kind: value
  value: "1"
---

Each cell of a vehicle's journey is costed as a fraction of full speed, drawn from the [terrain table](/systems/movement-and-terrain/#the-terrain-table) column its [`SpeedType`](/keys/speedtype/) selects. That fraction is multiplied by this value when the ground under the destination cell stands higher than the ground under the vehicle.

```ini title="rules.ini"
[General]
WheeledUphill=0.5
```

A step of two or more terrain levels is costed from the Road row of that table rather than from the destination's own land type. Ground heights are what is compared, so a ramp counts as a climb exactly as a cliff edge does. A step between cells of equal height takes neither this multiplier nor [`WheeledDownhill`](/keys/wheeleddownhill/).

The name is narrower than the rule. What gates the multiplier is the kind of object: only vehicles reach it, so infantry and aircraft take neither pair. Among vehicles, only `SpeedType=Track` is sorted off to [`TrackedUphill`](/keys/trackeduphill/); every other SpeedType a vehicle can use (`Wheel`, `Hover`, `Float`, `Amphibious`, `Foot`, `Creep`, `Winged`) climbs on this value. A vehicle whose [`Locomotor`](/keys/locomotor/) hovers or tunnels rather than driving moves by another path, and the multiplier never reaches it.

:::caution[The multiplier is applied after the terrain cost is capped]
The terrain cost is clamped to full speed first and this value is applied afterwards, so a figure above `1` moves a vehicle uphill faster than it moves on the flat. A product of exactly zero is then replaced with half speed, so `WheeledUphill=0` halves a climb rather than forbidding one.
:::
