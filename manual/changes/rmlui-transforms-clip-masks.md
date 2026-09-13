---
title: Draw transformed and rounded-clipping UI documents
category: internal
release: 0.2.0
targets:
- type: system
  id: ui-files
  effect: changed
credit:
- ZivDero
---

The UI renderer draws two effects it used to refuse. A document may now transform an element, and may clip content under a rounded corner or inside a transformed ancestor; both are ordinary in interface styling, and a document that reached either one before opened its Win32 dialog instead.

Transforms travel to the renderer as the matrix RmlUi builds, and a clip mask is written to the stencil the back buffer already carries, so neither needs a shader or an off-screen target. Clip masks may nest as deep as eight bits count.

A document's images are also sampled with wrap rather than clamp, which is what a tiled decorator repeats through, and with the filter the game frame under them was magnified by.

Layers, filters and shaders are still refused, and with them `box-shadow`, which RmlUi renders through a layer.
