---
key: WallOwner
scope: buildingtype
label: Wall-claiming structure
see_also: ["system:walls-and-gates"]
when_omitted:
  kind: value
  value: "yes"
---

`WallOwner=no` keeps that type's buildings out of the search that gives the walls in a map's overlay data an owner. They stay out even when their owner's country allows wall ownership.

A building claims map walls only when its owner's country also allows it. [`WallOwner`](/keys/wallowner/#scope-housetype) on the country decides that.
