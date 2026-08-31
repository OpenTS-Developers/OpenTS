---
title: Play each campaign's own intro
category: fix
release: 0.2.0
targets:
- type: system
  id: campaign-progression
  effect: changed
credit: [ZivDero]
---

A campaign now opens with `INTR<n>.VQA`, where `n` is its `CD` number, so each campaign
reaches its own introduction instead of every campaign reaching the first one installed. A
deployment holding a single `INTRO.VQA` is unaffected, because that is what plays when no
numbered file is found.
