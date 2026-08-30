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

The connection that delivered a command owns its event origin. Power,
archive-target, repair, primary-factory, mission, idle, deploy, scatter, and
sell commands also require their object to belong to the sender when the
command executes, so capturing it while the command is in flight does not
transfer control. Network timing events require the session master. Player
removal requires either the current master or, when the master is leaving, the
first remaining human player in house order.

In-game chat, progress, sign-off, ready, and kick packets now have to come from
one uniquely matched address in the match's player list. An exact IP address
and port wins; one same-IP entry with a stored port of zero is the legacy
fallback. Ambiguous matches are rejected. Chat identity and kick votes are
taken from that membership record, so changing the corresponding fields in a
packet cannot impersonate another player.

Address matching is roster attribution, not authentication: this global sender
check neither pins nor updates the matched address, and a participant can still
disrupt a lockstep match. These checks do not change the existing packet
layouts or event IDs. All players should still use the same OpenTS snapshot.
