---
title: Damage a unit at the laser fence cell it enters
category: fix
release: 0.3.0
targets:
- type: system
  id: laser-fences
  effect: changed
credit:
- JoyfulShush
---

A vehicle, aircraft or infantryman that finishes a move in a cell whose laser fence segment is live is destroyed there. Before, an object that was not fully cloaked and finished a move in a row with a horizontal trigger line could be destroyed by a segment at the row's far end. It could also escape the live segment in its own cell.
