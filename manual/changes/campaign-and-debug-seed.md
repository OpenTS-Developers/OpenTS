---
title: Honor the seed a campaign launch or -SEED names
category: fix
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
- type: command
  id: launch:seed
  effect: changed
credit: [ZivDero]
---

A campaign mission started from a launch file now uses the file's `Seed`, as a skirmish already did, so launching the same file repeats the same random draws. It used to draw a new seed from the clock at every launch. The debug build's `-SEED<number>` option now reads its number; it used to start one character early and always read `0`, which draws a new seed.
