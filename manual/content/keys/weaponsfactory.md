---
key: WeaponsFactory
summary: Sends a finished object out through a door sequence instead of an ordinary exit cell.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "no"
---

The object is put down at a fixed coordinate inside the structure's footprint rather than on a searched exit cell. The structure then switches to its unload mission, which opens the door, clears whatever is standing on the exit cell and the cells around it, drives the object out and closes again.

Two exceptions to the ordinary exit rules follow. The structure is exempt from the one-object-at-a-time rule that makes a factory still in contact with its last object refuse the next attempt. While it is still unloading, it looks for another idle structure of the same type owned by the same house and lends it the production slot, so that structure lets the object out instead. With none free, the attempt is reported as a temporary blockage and retried. [Leaving the factory](/systems/production/#leaving-the-factory) covers what each outcome does to the order.

A carryall is also refused the pick-up cursor over an object standing on such a structure.
