---
key: LeptonsPerSightIncrease
summary: Height in leptons that earns ten percent more sight range per step.
see_also: ["system:map-visibility", Sight, VeteranSight, EliteSight]
when_omitted:
  kind: value
  value: "50"
---

An object sees ten percent further for each whole multiple of this value in the height the engine records for it. Height counts from the map's lowest level, so standing on a hill counts. One height level is 104 leptons. Lower values make height worth more.

| Value | One level up | Four levels up |
| --- | --- | --- |
| `50` | 20 percent further | 80 percent further |
| `26` | 40 percent further | 160 percent further |

The bonus multiplies the sight range for the object's rank, which is [`Sight=`](/keys/sight/) or a positive [`VeteranSight`](/keys/veteransight/#scope-aircrafttype) or [`EliteSight`](/keys/elitesight/), before the `SIGHT` ability multiplier applies. The result is rounded down to whole cells. A short sight range can therefore gain nothing: `Sight=3` one level up comes to 3.6 and still reveals 3 cells. [Sight range](/systems/map-visibility/#sight-range) gives the full calculation and its ten-cell cap.

A vehicle, infantryman or structure uses its current height, checked on every look. An aircraft uses the height where it was placed and keeps that figure while it flies, so taking off or climbing does not raise it.

:::danger[Keep the value above zero]
`LeptonsPerSightIncrease=0` divides by zero and crashes the game as soon as the first vehicle, infantryman, aircraft or structure is placed on the map.
:::
