---
title: Stop requiring the score and movie archives
category: fix
release: 0.2.0
targets:
- type: format
  id: mix
  effect: changed
credit: [ZivDero, FunkyFr3sh]
---

The game no longer refuses to start when `SCORES.MIX` or every `MOVIES*.MIX` is missing. Both are still mounted wherever they are found. A music track or movie whose file is not found in any archive or folder is skipped.

FunkyFr3sh is credited for the ts-patches change this follows.
