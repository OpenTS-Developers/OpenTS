---
key: SortCameoAsBaseDefense
summary: Sorts a BuildingType's cameo with the base defenses at the end of the structures strip.
see_also: ["system:sidebar", "SidebarSorting"]
when_omitted:
  kind: inherited
  note: "Follows [`IsBaseDefense=`](/keys/isbasedefense/#scope-buildingtype)."
---

The defense group is the last of the four the structures strip is arranged in;
[the order of the strips](/systems/sidebar/#the-order-of-the-strips) gives them all. A wall or a
gate is settled by its own flags first, so this key moves neither.

A structure the computer plans its defenses by carries [`IsBaseDefense=`](/keys/isbasedefense/#scope-buildingtype), and a mod that set
that flag to steer the computer can write `SortCameoAsBaseDefense=no` to leave the cameo among
the ordinary structures.

[`SidebarSorting=no`](/keys/sidebarsorting/) leaves the strips unsorted, and nothing reads this
key.
