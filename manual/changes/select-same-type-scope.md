---
title: Widen Select Same Type to every selected type and the whole map
category: feature
release: 0.1.0
targets:
- type: command
  id: SelectType
  effect: changed
credit: [JoyfulShush, ZivDero]
---

Select Same Type now adds to the selection rather than replacing it; it used to drop everything already selected before hunting for matches, so units standing off screen were lost from a selection that was only meant to grow. Only types taken from the player's own units count now, where a selection that included an enemy or neutral object swept the view for the player's units of that object's type.

Pressing the command a second time within half a second widens the sweep from the visible view to the whole map. A slower second press sweeps the view again, as a single press does.
