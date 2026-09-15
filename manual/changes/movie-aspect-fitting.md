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

A full screen movie played with `StretchMovies=yes` in `sun.ini` was scaled by width alone and then squeezed into the height the display actually had. The picture came out flattened wherever the display was proportionally wider than the movie: a 640 by 400 movie covered the whole of a 1920 by 1080 display. It now keeps its shape, playing at 1728 by 1080 with a black band down each side. A display whose shape already suited the movie, such as 1280 by 800, is unaffected.

The screen is cleared ahead of any full screen movie that will not cover the display, stretched or not. The bands around it are black rather than whatever the display last held.

CCHyper is credited for the Vinifera fix this one follows.
