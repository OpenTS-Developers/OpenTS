---
title: Draw the ion blast's warp to the view's edge
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

An ion cannon blast against the bottom of the view left an undisturbed band there: the drawer kept a seven-row margin above the bottom, which the original code needed to keep the warp's fetches inside the view. The fetches are now held inside the view where they are taken, so the margin is gone and the warp reaches the edge. In the lowest rows a fetch from below would leave the view, so those pixels are left alone and the warp quiets toward the edge.
