---
title: Place sound effects in the view
category: feature
release: 0.2.0
targets:
- type: format
  id: sound-ini
  effect: changed
- type: system
  id: sound-effects
  effect: added
credit: [ZivDero]
---

A sound played at a place in the world is now panned by where that place is across the view and fades with its distance from the view's edge, vertical distance counting double, over the sound's `Range=`. It keeps following the view while it plays, so scrolling away from a sound quietens it instead of leaving it as loud as it started. Up to sixteen sound effects play at once, and a new one displaces the lowest-priority one only when the budget is full.
