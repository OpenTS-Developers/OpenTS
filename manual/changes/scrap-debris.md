---
title: Honour the scrap debris option
category: feature
release: 0.2.0
targets:
- type: key
  id: ScrapExplosion
  effect: added
- type: key
  id: ScrapMetal
  effect: added
- type: format
  id: spawn-ini
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring]
---

`ScrapMetal=yes`, in a launch file or a scenario's `[SpecialFlags]`, makes a destroyed object leave the animations its type gives `ScrapExplosion=` rather than the ones it gives `Explosion=`. A type that names no scrap animations keeps its ordinary ones, so a ruleset can convert part of its arsenal at a time. The option reaches a campaign as readily as a match.
