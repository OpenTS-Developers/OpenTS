---
title: Reject malformed network packets
category: fix
release: 0.2.0
targets: []
credit:
- ZivDero
---

Malformed network traffic is rejected before it can enter the simulation.
Undersized and oversized envelopes, truncated events, invalid indices, and
packets claiming another player's identity no longer reach the state they
could crash or corrupt. A rejected command packet can still make an
uncooperative peer stall a lockstep match, but it cannot make the receiver use
bytes outside that packet.

In-game chat, progress, sign-off, ready, and kick packets now have to come from
an address in the match's player list. Chat identity and kick votes are taken
from that membership record, so changing the corresponding fields in a packet
cannot impersonate another player.

The packet layout used before this change remains accepted. All players should
still use the same OpenTS snapshot.
