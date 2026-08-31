---
title: Reject malformed network packets
category: fix
release: 0.2.0
targets: []
credit:
- ZivDero
---

Malformed, oversized, truncated, and misattributed network packets are rejected
before they change peer state or enter the simulation queue. Packet layouts and
event IDs are unchanged; all players must use the same OpenTS snapshot.
