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
commands require the object to still belong to that sender; network timing is
master-only, and player removal uses deterministic master-or-successor
authority.

In-game global controls require one unique roster endpoint, preferring an exact
IP/port match over one same-IP zero-port fallback. Roster identity supplies chat
and kick attribution. This is not authentication or complete cheat prevention,
and it does not change existing packet layouts or event IDs. All players must
use the same OpenTS snapshot.
