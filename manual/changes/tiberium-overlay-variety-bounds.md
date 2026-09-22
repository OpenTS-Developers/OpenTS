---
title: Keep a Tiberium overlay inside its own set
category: fix
release: 0.2.0
targets:
- type: system
  id: tiberium
  effect: changed
credit:
- ZivDero
---

Seeding a bare cell now draws its overlay from the number of flat overlays the Tiberium type has, rather than from a fixed twelve. Every `Image=` value sets that number to twelve, so no rules can tell the difference; the fixed figure was simply not the one the rest of the system reads.

A Tiberium type whose overlay set carries no slope artwork also draws nothing on a slope, instead of dividing by the number of slope overlays it does not have. Spreading already refused such ground, so the cells that reach this are the ones a map or a third-party editor placed.
