---
title: Keep fogged object redraws inside safe bounds
category: fix
release: 0.2.0
credit: [Krisztiaan]
targets:
- type: system
  id: map-visibility
  effect: changed
---

A fog-of-war scenario could stop the game while loading or playing. A cell's redraw area extends beyond its own diamond, so a fogged structure, terrain object, overlay or smudge at the edge of the tactical view passed an out-of-frame clipping window to the shape renderer, which then addressed pixels outside the frame. These are now clipped to the view.

Fogged buildings that use their owner's palette also reuse that owner's color converter, matching visible buildings. They treated the map cell as a color converter, which could overwrite the cell with rendering state and stop the game during a later terrain-overlay redraw.
