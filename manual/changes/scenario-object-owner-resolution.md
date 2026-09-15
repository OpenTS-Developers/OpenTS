---
title: Skip scenario records without a live owner
category: fix
release: 0.1.0
targets:
- type: format
  id: scenario-objects
  effect: changed
credit: [ZivDero]
---

Scenario object rows and trigger definitions now require their registered HouseType to have a live house in the current session. A row without one is skipped before construction, and a link to a missing or rejected trigger stays empty, so a tag naming one remains inert. Vehicle follower IDs are resolved through the original `[Units]` row positions. Skipping or failing to place a row no longer redirects another vehicle's follower link through a compacted runtime list.
