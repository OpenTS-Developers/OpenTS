---
title: Let a type say it is immune to an EM pulse
category: feature
release: 0.2.0
targets:
- type: key
  id: ImmuneToEMP
  effect: added
- type: key
  id: IsCoreDefender
  scope: buildingtype
  effect: changed
- type: key
  id: IsCoreDefender
  scope: unittype
  effect: changed
- type: system
  id: emp-pulse
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring]
---

`ImmuneToEMP=yes` on an object type makes an EM pulse pass it by: a landed aircraft is not wrecked, a unit is not stunned, a structure is not powered off, and a limpet mine is not destroyed. The object still springs its Paralyzed trigger event.

`IsCoreDefender=yes` on a structure or vehicle now only supplies this key's default, so Firestorm's Core Defender keeps its immunity and a ruleset can switch it off with `ImmuneToEMP=no`.

Rampastring is credited for the Vinifera feature this follows.
