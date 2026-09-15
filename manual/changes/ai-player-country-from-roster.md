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

A computer player the menu seats is now given one of the countries the lobby offers, drawn at random from those with `Multiplay=yes`. It previously drew from the first two countries in the rules, whatever they were. The shipped rules mark two countries `Multiplay=yes`, and both are among the first two. A game on the shipped rules draws as it did before.
