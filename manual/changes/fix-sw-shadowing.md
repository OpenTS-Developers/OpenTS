---
title: Keep superweapons that share an Action from replacing each other
category: fix
release: 0.2.0
targets:
- type: system
  id: superweapons
  effect: changed
credit: [Templarfreak]
---

Placing a superweapon on the map now fires the one that was actually armed, even when
another superweapon defines the same Action. Clicking a target cell previously resolved
back to whichever superweapon type first matched that Action, so two superweapons sharing
one Action value could not be fired independently: placing either one always discharged
the same type and left the other's charge untouched.

The game now remembers the specific superweapon that was armed from the sidebar and fires
that one directly, falling back to the old Action lookup only when nothing was armed
through the sidebar.
