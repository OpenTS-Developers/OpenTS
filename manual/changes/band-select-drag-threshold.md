---
title: Take the band-selection threshold from the system
category: fix
release: 0.2.0
targets:
- type: system
  id: band-selection
  effect: changed
credit: [ZivDero, dkeeton]
---

A selection box now starts once the pointer has moved the drag distance Windows reports, on either axis, instead of four pixels in any direction. The distance follows the display scale and the pointer accessibility settings; on a standard display it is still four pixels.
