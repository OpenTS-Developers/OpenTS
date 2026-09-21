---
title: Let a building be placed over an overlay
category: feature
release: 0.2.0
targets:
- type: key
  id: BuildableOver
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring]
---

`BuildableOver=yes` on an OverlayType lets a building's foundation cover that overlay, for the player and for computer houses. The cell must still allow building on the land type the overlay gives it, and a wall refuses a foundation whatever the setting.
