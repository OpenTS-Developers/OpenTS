---
title: Correct sprite depth and lighting
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

Some translucent and remapped sprites could overlap other objects incorrectly because their depth checks and updates used values from different pixels. Those draw paths now keep depth values aligned with the pixels they draw. Remapped sprites with alpha lighting and depth updates now use each pixel's lighting instead of repeating the first pixel's.
