---
title: Continuous keyboard map scrolling
category: feature
release: 0.2.0
targets:
- type: command
  id: ScrollNorth
  effect: changed
- type: command
  id: ScrollSouth
  effect: changed
- type: command
  id: ScrollEast
  effect: changed
- type: command
  id: ScrollWest
  effect: changed
credit:
- torradmin
---

Binding "Scroll East/West/North/South" to a key now scrolls the map continuously while the key
is held, the way edge scrolling and mouse-drag scrolling already do; before, each press moved
the map a single fixed step and repeating it required releasing and pressing the key again,
because Windows key-repeat events never reach the game's keyboard buffer. Held keyboard scroll
ramps up to the same top speed as edge scrolling and honors the "Scroll speed" option.
