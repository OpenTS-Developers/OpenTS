---
title: Ask the whole war factory list who owns one
category: fix
release: 0.2.0
targets:
- type: key
  id: BuildWeapons
  effect: changed
credit: [ZivDero, AlexB]
---

A house selling its base back to afford a harvester now counts every `BuildWeapons` entry as
a war factory. It asked only the first two, and asked for the second without checking that
the list had one, so a one-entry list was read past its end. The two places that already asked
the whole list are unchanged.

AlexB is credited for the ts-patches bundle that first read these lists whole.
