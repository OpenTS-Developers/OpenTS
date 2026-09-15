---
title: Show indicators on hover and rank without selection
category: feature
release: 0.2.0
targets:
- type: system
  id: veterancy
  effect: changed
- type: key
  id: EnemyHealth
  effect: changed
credit:
- ZivDero
- AlexB
- dkeeton
---

The object under the mouse pointer now draws its condition indicator, which used to want a selection. Veterancy insignia and the healer's cross stand without one, for a viewer allied to the object's owner or spying on that house. Both obey the same visibility rule the pointer applies, so shroud, fog and an unsensed cloak hide an indicator with the object it belongs to. `EnemyHealth`, in `[AudioVisual]` of `rules.ini`, does not gate either indicator. Cargo pips, the group number and the "Primary" tag still wait for a selection.
