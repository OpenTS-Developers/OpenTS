---
key: Crushable
scope: aircrafttype
label: Crushable by vehicles
see_also: ["system:walls-and-gates", "CrushSound", "Crusher"]
when_omitted:
  kind: context-dependent
  note: An InfantryType section starts at yes. Every other object type in this scope starts at no.
---

The crusher on the other side of this setting is a [`Crusher=yes`](/keys/crusher/) UnitType or a unit whose type lists the `CRUSHER` ability on [`VeteranAbilities=`](/keys/veteranabilities/) or [`EliteAbilities=`](/keys/eliteabilities/) and has been promoted to the matching rank. Everything below applies to that pairing only; nothing else in the game destroys the object or overlay over it, though the flag does shape pathfinding and the track a driving vehicle takes.

**An overlay** marked crushable and [`Wall=yes`](/keys/wall/) is destroyed outright the moment such a vehicle drives onto it. The cell plays the overlay's [`CrushSound`](/keys/crushsound/#scope-aircrafttype), the wall segment is removed with no regard for its owner or its damage stage, and the vehicle rocks forward. A crushable overlay that is not a wall gets the same sound and the same rocking but survives. The removal step is written for wall segments and does nothing to it. The cell also reports itself as crushable terrain, which lets a crusher's pathfinding route straight through a crushable wall. A wall owned by an ally is also flagged as friendly and destroyable.

**An object** marked crushable is run over rather than shot. A crusher that finds one in the cell it is entering deletes it, plays its `CrushSound`, and records the kill. Allied objects are passed over unharmed unless the crusher is a train. A computer-controlled crusher drives at a crushable target that is inside the `[CombatDamage] Crush=` distance instead of firing on it. [`AutoCrush=yes`](/keys/autocrush/) and `[CombatDamage] PlayerAutoCrush=` are tested only on branches a house a human is playing never reaches. An unarmed crusher pointed at a crushable object is offered a move cursor in place of the attack cursor it could not use.
