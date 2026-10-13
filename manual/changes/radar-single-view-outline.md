---
title: Draw one white outline on the radar
category: fix
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
credit:
- equalchance
---

The radar used to draw a white frame around the whole map image as well as the white rectangle that marks the tactical view. When the view covered much of a small map, the two looked like two view outlines. The radar now draws only the view rectangle.
