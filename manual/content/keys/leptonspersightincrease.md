---
key: LeptonsPerSightIncrease
summary: Leptons of height that earn an object ten per cent more sight range.
see_also: ["system:map-visibility", Sight, VeteranSight]
when_omitted:
  kind: value
  value: "50"
---

The elevation of the object's own coordinate is divided by this value and truncated to a whole number. Each whole unit of that result adds ten per cent to the type's [`Sight=`](/keys/sight/), and the veteran multiplier is applied after that bonus. One height level is 104 leptons. At the default an object one level up sees a fifth further, and an object four levels up sees eighty per cent further. Lowering it to `26` makes a height level worth forty per cent.

The bonus is recomputed on every look and again when an object is placed on the map. A rise in it makes that same look a full scan rather than an incremental one.

:::danger[Zero divides by zero]
The height is divided by this value with no guard against zero. `LeptonsPerSightIncrease=0` crashes the game on the next look any object takes.
:::
