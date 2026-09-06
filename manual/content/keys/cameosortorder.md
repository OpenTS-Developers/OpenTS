---
key: CameoSortOrder
summary: Places a type's sidebar cameo within its category, ahead of the order the strip would otherwise give it.
see_also: ["system:sidebar", "SidebarSorting"]
---

The lowest number comes first and negative numbers are accepted. Types sharing a number fall
back on [the order of the strips](/systems/sidebar/#the-order-of-the-strips), so leaving every
type at zero arranges the strip entirely by that order.

A cameo cannot be placed between two types that both keep the default; number both neighbours
instead. A full strip still takes the types that reached it first, so a number cannot win a
cameo a place on one.

[`SidebarSorting=no`](/keys/sidebarsorting/) leaves the strips unsorted, and nothing reads this
key.
