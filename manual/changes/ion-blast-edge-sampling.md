---
title: Hold the ion blast's warp inside the view
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

An ion cannon blast near the edge of the view no longer risks a crash at the highest detail level. The shockwave warps the scene by replacing each pixel it covers with a neighbor. That neighbor is fetched up to eight pixels sideways and sixteen rows up or down. A blast close enough to the view's edge could fetch it from past the end of the frame, or from the interface beside the view. A fetch that would come from outside the view is now refused, and that pixel keeps the color the scene gave it. The warp still reaches every pixel it always did.
