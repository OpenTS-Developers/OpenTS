---
title: Adapt private network retries
category: performance
release: 0.2.0
targets:
- type: system
  id: network-transport-timing
  effect: added
credit:
- ZivDero
---

Each private connection estimates its own round trip and doubles the wait
between repeated transmissions. Its retry timeout doubles with them, so a link
whose latency climbs above that timeout can still be measured. A packet that
reaches the connection timeout keeps retrying while the receive queue keeps
freeing space, so a recovered link drains its backlog. Global lobby traffic
keeps its fixed retry cadence. Packet layouts, event IDs, and configuration are
unchanged.
