---
title: Declare weapons in the rules
category: feature
release: 0.2.0
targets:
- type: format
  id: rules-registries
  effect: changed
credit: [ZivDero, CCHyper]
---

A `[Weapons]` list in `rules.ini` registers the weapons it names, in the order it writes them, before anything else in the rules can name one. Tiberian Sun ships such a list. Weapons used to be registered only as an object type, a superweapon, `DropPodWeapon=` or a projectile's `AirburstWeapon=` named them. That order is the numbering a map's Do Explosion At trigger action counts through, so the numbers those actions store move wherever a rules file declares the list.

A weapon the list names now reads its own section even where nothing else names it. A weapon only a projectile's `AirburstWeapon=` names is created too late to read its own section, so it keeps the built-in defaults: no projectile, no warhead, no damage.

Rules declaring no list keep the old order, with every weapon registered the first time something names it.
