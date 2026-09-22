---
title: Build a random map when a map file asks for one
category: feature
release: 0.2.0
targets:
- type: key
  id: RandomMap
  effect: added
- type: format
  id: spawn-ini
  effect: changed
- type: system
  id: map-generation
  effect: changed
credit:
- ZivDero
- dkeeton
---

A scenario file with `RandomMap=yes` in `[Basic]` is now generated from its `[RandomMap]` section, like a `.SED` seed file, using the match's seed. Every machine builds the same map, and a new seed builds a new one. The file's other sections still apply, such as its rules sections and `FreeRadar`.

dkeeton is credited for the ts-patches patch that first let a map file ask for a random map.
