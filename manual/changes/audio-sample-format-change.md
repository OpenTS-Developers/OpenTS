---
title: Play samples whose format differs from the mixer's
category: fix
release: 0.2.0
targets:
- type: format
  id: aud
  effect: changed
credit:
- ZivDero
---

A sample did not play unless its rate, bit depth and channel count matched 22050 hertz sixteen bit mono. The sound channel it was given then stayed silent for every later sound. Such a sample now plays in its own format.
