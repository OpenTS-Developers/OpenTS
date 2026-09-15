---
title: Measure fuse proximity in three dimensions
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

A proximity-fused projectile now measures its distance to the target in full, height included. A hovering object with no destination no longer claims to be heading for the top-left corner of the map. The fuse compared only the horizontal distance, so a shot passing well above or below its target read as having arrived. The check for a shot that had already flown past worked from the same flattened figure.
