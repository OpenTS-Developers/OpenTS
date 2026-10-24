---
key: DropPodInfantryMaximum
summary: Upper bound for the number of infantry requested by the Drop Pods superweapon.
see_also: [DropPodInfantryMinimum, "system:drop-pods"]
---

Each firing of the Drop Pods superweapon picks a squad size at random from [`DropPodInfantryMinimum`](/keys/droppodinfantryminimum/) to this value, both included. The key affects only the superweapon's `E1`/`E2` squad, not [`Droppod=yes`](/keys/droppod-teamtype/) TeamTypes.

A maximum below the minimum still works, because the pick takes the lower of the two values as its lower bound.
