---
title: Draw the in-game tooltips as a UI document
category: feature
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
- type: system
  id: sidebar
  effect: changed
credit:
- ZivDero
---

The tooltips over the sidebar and the tactical view are now drawn from `gametip.rml` in the `ui` directory, in the game's tooltip font, colors and frame. A mod can restyle them in `gametip.rcss`, including drawing them in any of the game's other fonts.
