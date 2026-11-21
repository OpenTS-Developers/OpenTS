---
title: Let a fully cloaked unit trigger cell tags
category: feature
release: 0.3.0
targets:
- type: key
  id: CellTagsIgnoreStealth
  effect: added
- type: system
  id: trigger-springing
  effect: changed
- type: format
  id: save-games
  effect: changed
credit:
- JoyfulShush
---

`CellTagsIgnoreStealth=` in the `[General]` section of `rules.ini` sets whether a fully cloaked infantryman or vehicle triggers cell tags as it moves. At `no`, the object triggers cell entry, both line crossings and zone entry like a visible object; at `yes`, it triggers none of them. An object that is still cloaking or uncloaking triggers them whether the key is `yes` or `no`.
