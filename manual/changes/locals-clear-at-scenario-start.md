---
title: Clear local variables at the start of every scenario
category: fix
release: 0.3.0
targets:
- type: event
  id: TEVENT_LOCAL_SET
  effect: changed
- type: event
  id: TEVENT_LOCAL_CLEAR
  effect: changed
credit:
- ZivDero
---

A local variable the map gave no starting state kept the value that the previous scenario, or an earlier attempt at the same mission, left in it. Such a local, and one the map does not declare, is now clear when the scenario starts.
