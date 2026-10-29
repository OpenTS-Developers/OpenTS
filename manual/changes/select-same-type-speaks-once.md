---
title: Select Same Type speaks once for each type
category: fix
release: 0.2.0
targets:
- type: command
  id: SelectType
  effect: changed
- type: event
  id: TEVENT_SELECTED
  effect: changed
credit: [fjlaubscher]
---

Select Same Type used to play a selection response for every object it added, and offer Selected by player for each of those objects. It now plays one response for each type it adds, from the first object of that type, and offers Selected by player for that object only.
