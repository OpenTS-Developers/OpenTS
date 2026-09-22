---
title: Fix Tiberium on the large-crystal set and on slopes
category: fix
release: 0.2.0
targets:
- type: system
  id: tiberium
  effect: changed
- type: key
  id: Image
  scope: tiberium
  effect: changed
credit:
- ZivDero
- Rampastring
- dkeeton
---

A Tiberium type on `Image=2` now has twelve growth stages instead of one, so it can grow, spread and yield several loads per cell. The stock `Cruentus` still neither grows nor spreads, and a mod that wants the set harvested must also give its overlays a land type harvesting reads, since the shipped ones are rock.

That set has no slope artwork, so a type on it, including one moved there by a later file, now draws nothing on a slope. It used to divide by zero or draw overlays from outside its set.

Tiberium a map places on a corner, steep or double slope is now removed when the map loads. It used to be drawn with another set's overlay and could crash the game. No shipped map places Tiberium there.

Rampastring and dkeeton are credited for the ts-patches fix that gave the large-crystal set twelve stages.
