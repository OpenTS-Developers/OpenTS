---
title: Draw the sonic ripple to the view's edge
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

A sonic wave lying against the edge of the view left an unrippled band there: the drawing walk kept a margin of three pixels on two sides and two on the others, which the original code needed to keep the ripple's replacement samples inside the view. The samples are now held inside the view where they are taken, so the margin is gone and the ripple reaches the edge. In the outermost rows a full lift would leave the view, so those pixels keep their own color and take only the wave's tint, letting the ripple fade at the edge rather than stop short of it.
