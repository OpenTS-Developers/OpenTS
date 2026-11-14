---
title: Give a gate type its own opening and closing sounds
category: feature
release: 0.2.0
targets:
- type: key
  id: GateUpSound
  effect: added
- type: key
  id: GateDownSound
  effect: added
- type: format
  id: save-games
  effect: changed
credit: 
  - JoyfulShush
  - CCHyper
---

`GateUpSound=` and `GateDownSound=` in a gate's section in `rules.ini` name the sounds its door plays as it starts closing and as it starts opening from rest. Those sounds play instead of the `[AudioVisual]` `GateUp=` or `GateDown=` sound if they are defined.

Credits to CCHyper for their original Vinifera implementation for this feature.
