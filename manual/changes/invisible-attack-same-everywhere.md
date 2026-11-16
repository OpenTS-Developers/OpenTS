---
title: Decide attacks on invisible objects the same way on every machine
category: fix
release: 0.3.0
targets:
- type: key
  id: Invisible
  effect: changed
credit:
- ZivDero
---

Whether a weapon could fire at an `Invisible=yes` object, and when such an object with a cloaking device finished cloaking or uncloaking, depended on whether the local player owned it. This put a multiplayer game out of sync, and in a campaign mission or skirmish it let computer players attack the player's invisible objects. Now every machine refuses a damaging attack by any house but the owner unless a sensor of the attacking house covers the object's cell, and the object cloaks and uncloaks like any other cloaking object.
