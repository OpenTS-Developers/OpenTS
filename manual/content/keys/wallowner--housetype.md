---
key: WallOwner
scope: housetype
label: Wall-claiming country
see_also: ["system:walls-and-gates"]
when_omitted:
  kind: value
  value: "yes"
---

`WallOwner=no` keeps every building of that country out of the search that gives the walls in a map's overlay data an owner. Each wall then goes to the nearest building that does qualify; if no building qualifies anywhere, the wall stays unowned.

A building also needs its own type to allow wall ownership. [`WallOwner`](/keys/wallowner/#scope-buildingtype) on a structure type keeps that type out even when its country allows it.
