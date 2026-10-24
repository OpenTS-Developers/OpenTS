---
key: DropPodInfantryMinimum
summary: Lower bound for the number of infantry requested by the Drop Pods superweapon.
see_also: [DropPodInfantryMaximum, "system:drop-pods"]
---

Each firing of the Drop Pods superweapon picks a squad size at random from this value to [`DropPodInfantryMaximum`](/keys/droppodinfantrymaximum/), both included. The key affects only the superweapon's `E1`/`E2` squad, not [`Droppod=yes`](/keys/droppod-teamtype/) TeamTypes.

The squad that lands can be smaller than the size picked. The superweapon allows three placement attempts per soldier, pooled across the squad, and discards a soldier whose attempt fails. [Drop Pods superweapon](/systems/drop-pods/#drop-pods-superweapon) explains when an attempt fails. A size of `0` drops nothing.

Keep both values at `0` or above. If the size picked comes out negative, the superweapon never finishes placing its squad and the game stops responding.
