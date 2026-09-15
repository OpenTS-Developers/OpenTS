---
key: PowersUpBuilding
summary: The ObjectType ID of the structure this type plugs into as an upgrade.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: none
---

A type that names a host is placed onto an existing structure instead of onto the ground. The placement is accepted when the target cell holds a structure of the named type, matched without regard to letter case. The host must be owned by the same house and have room for the plug. A plug whose [`PowersUpToLevel`](/keys/powersuptolevel/) is `-1` needs one free [upgrade slot](/keys/upgrades/); a value of `1`, `2` or `3` is accepted only on a host with no plugs at all. The plug is then absorbed: its own object is deleted and the host records it in that slot. A host that is full, of the wrong type, or owned by someone else refuses the placement.

Once absorbed, the plug's [`Power=`](/keys/power/#scope-buildingtype) is added to the host's contribution before the host's health scales it, so a turbine on a half-destroyed plant delivers half of its own rating too.

:::caution[Installing a plug rewrites the host type's animation slot]
The plug's art name is copied into the host BuildingType's animation entry for the slot being filled. The entry belongs to the type, not to the structure, so every later host of that type shows the art of whichever plug was installed last.
:::
