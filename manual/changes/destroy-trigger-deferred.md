---
title: Destroy triggers and tags at the end of the frame
category: fix
release: 0.1.0
targets:
- type: action
  id: TACTION_DESTROY_TRIGGER
  effect: changed
- type: action
  id: TACTION_DESTROY_TAG
  effect: changed
credit: [ZivDero, tomsons26]
---

Destroying a trigger or a tag now marks it and lets the frame finish before the object is freed. Everything else in the scenario logic retires an object the same way. The two actions used to free each match immediately while still walking the list they were freeing from. A map that destroyed several at once could read past the end of that list, and a trigger or tag that destroyed its own type pulled itself out from under the action that was still running. A marked trigger stops springing the moment the action runs, so the point at which a map stops seeing it is unchanged.
