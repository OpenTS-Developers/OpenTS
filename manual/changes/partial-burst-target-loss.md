---
title: Preserve partial bursts across target loss
category: fix
release: 0.1.0
targets:
- type: key
  id: Burst
  effect: changed
credit: [ZivDero]
---

Losing a target now preserves a partially fired burst for one full rearm interval. Acquiring a valid target before that interval expires continues the same burst. Remaining without a target resets the next shot to the start of the burst when the interval ends.
