---
key: NonVehicle
summary: Stops a vehicle counting as a vehicle for repair weapons and for infantry that take vehicles over.
see_also: ["SmallVisceroid", "LargeVisceroid", "Jellyfish"]
when_omitted:
  kind: value
  value: "no"
---

The flag is narrower than its name. The type is still a UnitType and is still counted against its owner's vehicle total, so neither its registration nor the tallies the house keeps are touched. What it withdraws from is the handful of decisions that ask an object whether it is a vehicle at all.

Two of those decisions matter in a match. A weapon whose [`Damage`](/keys/damage/) is negative (a repair weapon) will not select the type as a target and is refused permission to fire at it, so a mobile repair vehicle cannot mend it. And infantry that take vehicles over pass it by, in three places:

- the type draws no capture cursor;
- a thief loses its repair and capture cursors over the type, and its exemption to enter the cell the type occupies, so it may still chase such a vehicle and simply never take it;
- the takeover that fires when a capturing infantryman reaches the type's cell is gated on the same test.

:::caution[A visceroid flag sets it regardless]
[`SmallVisceroid=yes`](/keys/smallvisceroid/#scope-unittype) or [`LargeVisceroid=yes`](/keys/largevisceroid/#scope-unittype) forces the flag on after this key has been read, so `NonVehicle=no` in a visceroid's section does not survive the same pass.
:::
