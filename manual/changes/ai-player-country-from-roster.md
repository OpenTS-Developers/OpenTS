---
title: Draw a computer player's country from the lobby's own list
category: fix
release: 0.2.0
targets:
- type: key
  id: Multiplay
  effect: changed
credit: [ZivDero]
---

A computer player the menu seats is now given one of the countries the lobby offers, drawn at
random from those carrying `Multiplay=yes`, where it drew from the first two countries in the
rules whatever they were. With the two playable countries the shipped rules name, the draw is the
same one it always was.
