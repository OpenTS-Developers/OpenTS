---
key: Flat
scope: animtype
label: Animation ground plane
see_also: ["Tiled", "Surface"]
when_omitted:
  kind: value
  value: "no"
---

With the flag set, the animation's pixels are depth-tested against the ground plane instead of against the standing gradient everything else uses. The artwork reads as painted on the terrain, and whatever stands on the cell covers it. The same choice is made for the copy drawn under fog.

A [`Tiled=yes`](/keys/tiled/) animation takes a different drawing path that never reaches this flag.
