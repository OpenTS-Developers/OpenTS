---
title: Keep Play Sound Effect At sounds at their waypoint
category: feature
release: 0.2.0
targets:
- type: action
  id: TACTION_PLAY_SOUND_AT
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

A looping sound started by Play Sound Effect At now stays at its waypoint: it fades as the view scrolls away, falls silent out of range, and starts again without its attack when the waypoint comes back into view. Such sounds travel with a save and resume on load. A one-shot sound plays once, as before. Sounds attached to objects are kept in a table of their own that is saved as well; nothing attaches one yet.
