---
title: Honor a trigger's difficulty flags
category: fix
release: 0.1.0
breaking: true
migration:
- Check maps whose triggers carry `0` in one of their three difficulty fields. Such a trigger is now genuinely disabled at that difficulty; set the field to `1` to keep it taking part.
targets:
- type: action
  id: TACTION_ENABLE_TRIGGER
  effect: changed
credit: [ZivDero, tomsons26]
---

A trigger that a map switches off for a difficulty no longer takes part when the scenario is played at that difficulty. The three difficulty fields of a trigger record were only ever able to turn a difficulty on. A `0` in any of them did nothing, and every trigger ran at every difficulty. The fields now read as written, the trigger is disabled as it is created, and the enable-trigger action leaves a trigger the difficulty excludes alone. Campaign missions take the difficulty the player chose, and a skirmish or multiplayer game takes the one the lobby's computer skill sets. Saved games keep the flags they were stored with.
