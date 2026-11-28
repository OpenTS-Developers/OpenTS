---
title: Raise the Direct3D 11 and OpenGL hardware requirements
category: internal
release: 0.3.0
targets:
- type: key
  id: Renderer
  effect: changed
breaking: true
migration:
- If you forced Direct3D 11 or OpenGL and your hardware falls below its new minimum, set Renderer=0 under
  [Video] in SUN.INI to let the game choose a graphics interface.
credit:
- ZivDero
---

Direct3D 11 now requires feature level 11.0 or higher. OpenGL now requires version 4.3 or higher. Hardware below either minimum needs another renderer it supports.
