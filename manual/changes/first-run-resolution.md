---
title: Choose the first-run resolution from the display
category: feature
release: 0.2.0
breaking: false
targets:
- type: key
  id: ScreenWidth
  effect: changed
- type: key
  id: ScreenHeight
  effect: changed
credit:
- fjlaubscher
---

A first run with either key missing from `[Video]` in `SUN.INI`, or set to `-1`, now opens at the largest whole fraction of the primary display that keeps the picture within 960 by 540 and at least 640 by 400. A 1920 by 1080 or 3840 by 2160 display opens at 960 by 540, and a 1280 by 800 display at 640 by 400. It previously opened at 640 by 480.
