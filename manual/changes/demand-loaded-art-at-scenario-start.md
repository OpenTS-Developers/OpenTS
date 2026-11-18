---
title: Load uncached art when each scenario starts
category: fix
release: 0.3.0
targets:
- type: key
  id: DemandLoad
  scope: animtype
  effect: changed
- type: key
  id: DemandLoad
  scope: buildingtype
  effect: changed
- type: key
  id: DemandLoad
  scope: overlaytype
  effect: changed
- type: key
  id: DemandLoadBuildup
  effect: changed
- type: key
  id: FreeAfterPlaying
  effect: removed
- type: key
  id: FreeBuildup
  effect: removed
credit:
- ZivDero
---

Art that an `art.ini` section marks with `DemandLoad=yes` or `DemandLoadBuildup=yes` used to load the first time the game needed it, and drawing could be what needed it. An animation took its largest frame, and a frame count and loop end its section left unset, from its shape as the shape loaded, so in a network game the machines could disagree on how many frames it played or on the frame that made its crater, scorch mark or fires, and go out of sync. This art now loads on every machine when a scenario starts or a saved game is loaded, and the values it sets are taken then.

`FreeAfterPlaying=` in an animation's `art.ini` section and `FreeBuildup=` in a structure's are no longer read. They released this art after an animation of the type ended or a structure's construction finished, and the next use read the file again. The art now stays loaded until the next scenario or saved game is loaded.
