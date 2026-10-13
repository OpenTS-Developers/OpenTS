---
title: Draw the sidebar larger at high resolutions
category: feature
release: 0.2.0
targets:
- type: key
  id: UIScale
  effect: added
- type: system
  id: sidebar
  effect: changed
credit:
- equalchance
---

At high resolutions the sidebar, the top strip, messages and the in-game dialogs are now drawn two to four times larger, while the battlefield keeps one screen pixel per map pixel. [`UIScale`](/keys/uiscale/) chooses the size; by default it is 3x from 2160 rows and 1x below, so 1080p and 1440p look as before.
