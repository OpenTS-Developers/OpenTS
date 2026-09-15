---
command_id: SelectOneLess
---

Deselects one object from the selection and leaves the rest selected. Selection order stacks an object whose type has a nonzero [`Damage`](/keys/damage/#scope-weapontype) on its primary weapon at the front, newest first, and every other object at the back, oldest first. The command removes the last object in that order, so an unarmed object such as a harvester leaves before the armed objects in the same selection. Deselecting the object the view is following ends follow mode.
