---
title: Hold the sonic ripple's samples inside the view
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

Scrolling so that a sonic wave leaves the bottom or a side of the view no longer risks a crash. The ripple draws each pixel by lifting a replacement pixel from up to three pixels away along the wave's direction of travel. A wave lying against the view's edge could lift its replacement from past the end of the frame, or from the interface beside the view. A replacement that would come from outside the view is now left untaken, and the pixel keeps its own color.
