---
key: SiloDamage
summary: Draws a fill level on a building that stores Tiberium.
see_also: ["system:tiberium", "Storage"]
when_omitted:
  kind: value
  value: "no"
---

The building shows an extra animation whose frame is four times its stored amount over its own [`Storage`](/keys/storage/), rounded and clamped to the range 0 through 3. Frame 0 is not drawn: a building holding less than one-eighth of its capacity removes the animation instead. A building declaring the flag but no capacity holds the animation at 0 and so never shows it.
