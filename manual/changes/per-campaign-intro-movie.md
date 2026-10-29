---
title: Play each campaign's own intro
category: fix
release: 0.2.0
targets:
- type: system
  id: campaign-progression
  effect: changed
credit: [ZivDero, CCHyper, tomsons26]
---

A campaign's introduction is `INTR<n>.VQA` when that file is present, and otherwise `INTRO.VQA` from the movie archive for its disc, unless `CampaignIntroMovie=no` under `[siliconsun]` in `SUN.INI`, which leaves those archives in alphabetical order. Every campaign used to play the `INTRO.VQA` in the first movie archive, so Nod played the GDI introduction when both campaigns were installed. It still plays when the first mission starts, not when the mission restarts, and only when `CD=` is below `2`.

The main menu's Intro / Sneak Peek item now plays `INTR0.VQA` when that file is present, and `INTRO.VQA` from the first movie archive otherwise.

CCHyper and tomsons26 are credited for the Vinifera `IntroMovie` key, which reaches the same end by another route.
