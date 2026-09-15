---
key: CameoSortOrder
scope: aircrafttype
label: Cameo sort order on a buildable type
see_also: ["system:sidebar", "SidebarSorting"]
when_omitted:
  kind: value
  value: "0"
---

The lowest number comes first, and types sharing a number fall back on [the order of the strips](/systems/sidebar/#the-order-of-the-strips). The number ranks a cameo against the others of its own kind and against nothing outside it. A vehicle never sorts among the infantry however it is numbered. Within the structures strip it outranks the wall, gate and defense grouping, so a numbered building sorts against every other structure whichever group it belongs to.
