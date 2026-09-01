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

Select Same Type now adds to the selection rather than replacing it. The command dropped
everything that was selected before hunting for matches, so units standing off screen were
lost from a selection that was only meant to grow.

Units the player does not control no longer decide what is hunted for. A selection that
included an enemy or neutral object used to sweep the view for the player's own units of
that object's type; only types taken from the player's own units count now.

Pressing the command a second time within half a second widens the sweep from the visible
view to the whole map, which gathers every unit of the selected types wherever it stands.
A slower second press sweeps the view again, as a single press does.
