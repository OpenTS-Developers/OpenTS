---
title: Draw a tooltip that fills the width of its panel
category: fix
release: 0.1.0
targets:
- type: system
  id: sidebar
  effect: changed
credit:
- ZivDero
---

A tooltip whose text comes to the full width of the sidebar was drawn on neither the sidebar nor the tactical view. A tooltip is kept within whichever of the two it sits over, so one wide enough to reach the far edge is moved back until it ends exactly there, and the test that picks which panel to draw it on required it to begin past the boundary between them rather than at it. Such a tooltip now draws, where a build cameo whose name and price together came to the width of the sidebar showed nothing at all, however long the pointer rested on it.
