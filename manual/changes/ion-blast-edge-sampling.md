---
title: Hold the ion blast's warp inside the view
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

An ion blast's ripple near the edge of the view no longer risks a crash at the highest detail level. Pixels whose replacement would come from outside the view keep their original color.
