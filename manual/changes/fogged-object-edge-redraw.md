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

A fog-of-war scenario could stop the game while loading or playing. A cell's redraw area extends beyond its own diamond. A fogged structure, terrain object, overlay or smudge at the edge of the tactical view therefore passed an out-of-frame clipping window to the shape renderer, which then addressed pixels outside the frame. These are now clipped to the view.

Fogged buildings that use their owner's palette also draw with that owner's colors, matching visible buildings. They took those colors from the map cell instead, which could overwrite the cell with rendering state and stop the game during a later terrain-overlay redraw.
