---
title: Reject malformed network packets
category: fix
release: 0.2.0
targets: []
credit:
- ZivDero
---

Malformed, oversized, truncated, and misattributed network packets are rejected
before they change peer state or enter the simulation queue.

Private events inherit the delivering connection's identity. Covered object
commands require the object to still belong to that sender, and network timing
is master-only. Player removal remains a trusted-peer control; redundant events
keep simultaneous departures from stranding a human house.

In-game global controls require one unique roster endpoint, preferring an exact
IP/port match over one same-IP zero-port fallback. Roster identity supplies chat
and kick attribution. This is not authentication or complete cheat prevention,
and unknown private endpoints no longer rewrite roster addresses. Packet
layouts and event IDs are unchanged. All players must use the same OpenTS
snapshot.
