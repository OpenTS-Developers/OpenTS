---
title: Fit a stretched movie to the display without distorting it
category: fix
release: 0.2.0
targets:
- type: key
  id: StretchMovies
  effect: changed
credit: [ZivDero, CCHyper]
---

A full screen movie played with `StretchMovies=yes` was scaled by width alone and then squeezed into the height the display actually had, so on a display proportionally wider than the movie the picture came out flattened: a 640 by 400 movie covered the whole of a 1920 by 1080 display rather than playing at 1728 by 1080 with a black band down each side. It now keeps its shape. A display whose shape already suited the movie, such as 1024 by 768, is unaffected.

The screen is cleared ahead of any full screen movie that will not cover the display, stretched or not, so the bands around it are black rather than whatever the display last held.

CCHyper is credited for the Vinifera fix this one follows.
