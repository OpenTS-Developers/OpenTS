---
title: Offer scaled sizes in the display options
category: feature
release: 0.2.0
targets: []
credit:
- fjlaubscher
---

The display options now list only the primary display's own size and its exact whole fractions: the width and height divided by 2 through 6, kept only when both divide exactly and the result is still at least 640 by 400. A 1280 by 800 panel includes 640 by 400, and a 3840 by 2160 panel includes 1920 by 1080 and 960 by 540. Each entry is marked `(native)` or `(1/2)` and so on. Sizes the display reports besides its own size, and the size Windows display scaling corresponds to, are no longer offered. Choosing one of those sizes renders the picture at that size.
