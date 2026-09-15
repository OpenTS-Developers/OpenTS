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

Select Same Type now adds to the selection rather than replacing it. It used to drop everything already selected before hunting for matches, so units standing off screen were lost from a selection that was only meant to grow. The types hunted for now come only from the player's own units. A selection that included an enemy or neutral object used to sweep the view for the player's units of that object's type as well.

Pressing the command a second time within half a second widens the sweep from the visible view to the whole map. A slower second press sweeps the view again, as a single press does.
