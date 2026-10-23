---
title: Fix diagonal keyboard scrolling and modifier-bound scroll keys
category: fix
release: 0.2.0
targets:
- type: command
  id: ScrollNorthEast
  effect: changed
- type: command
  id: ScrollSouthEast
  effect: changed
- type: command
  id: ScrollSouthWest
  effect: changed
- type: command
  id: ScrollNorthWest
  effect: changed
credit:
- torradmin
---

Binding "Scroll Northeast/Southeast/Southwest/Northwest" to a key now scrolls the map
continuously while the key is held and honors the "Scroll speed" option, matching the cardinal
scroll commands and edge scrolling; before, each diagonal command moved the map a single fixed
step regardless of how long the key was held.

A repeatable scroll command bound with a Shift, Ctrl, or Alt modifier (for example Shift+Up) no
longer keeps scrolling once the modifier is released, and no longer fires alongside an unrelated
command bound to the same base key with a different modifier. The per-frame poll that re-triggers
held scroll keys was checking only the base key, ignoring any modifier the binding required.
