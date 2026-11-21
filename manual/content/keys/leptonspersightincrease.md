---
key: LeptonsPerSightIncrease
summary: Leptons of height that earn a vehicle, infantryman or structure ten percent more sight range.
see_also: ["system:map-visibility", Sight, VeteranSight]
when_omitted:
  kind: value
  value: "50"
---

A vehicle, infantryman or structure sees ten percent further for each whole multiple of this value in its height. Height counts from the map's lowest level, so standing on a hill counts. One height level is 104 leptons. Lower values make height worth more.

| Value | One level up | Four levels up |
| --- | --- | --- |
| `50` | 20 percent further | 80 percent further |
| `26` | 40 percent further | 160 percent further |

The bonus multiplies the type's [`Sight=`](/keys/sight/) before [`VeteranSight`](/keys/veteransight/) applies, and the result is rounded down to whole cells. A short sight range can therefore gain nothing: `Sight=3` one level up comes to 3.6 and still reveals 3 cells. [Sight range](/systems/map-visibility/#sight-range) gives the full calculation. Aircraft get no height bonus.

:::danger[Keep the value above zero]
`LeptonsPerSightIncrease=0` divides by zero and crashes the game as soon as the first vehicle, infantryman, aircraft or structure is placed on the map.
:::
