---
title: Network synchronization
summary: Keeps every machine on the same simulation frame while adapting command delay and retransmission timing to the measured links between the players.
category: multiplayer-networking
keys: []
---

Network games exchange commands rather than copies of the game state. Each
command names the simulation frame on which every machine executes it, and a
machine waits when advancing would carry it too far beyond a player whose
commands have not arrived. The look-ahead distance is therefore both the time
available for delivery and the delay before a player's command takes effect.

## Packet admission

Every synchronized-event packet passes through bounded transport, connection,
and event decoders. Its envelope is the leading `FRAMEINFO` or sole
`FRAMESYNC`, which carries sender and frame information. The whole event stream
must validate before anything enters the simulation queue.

Compressed events inherit the delivering connection's identity. Power,
archive-target, repair, primary-factory, mission, idle, deploy, scatter, and
sell events also require their object to still belong to that sender. Missing
or destroyed objects remain no-ops; captured objects are rejected.

Network timing is master-only. The master removes other players; the first
remaining network-human house in house order removes a departing master. Each
machine recomputes that authority when the event executes.

Public discovery remains public. In-game chat, progress, sign-off, ready, and
kick controls require one unique roster endpoint: an exact IP/port match, or
one same-IP entry whose stored port is zero. Roster identity supplies chat and
kick attribution. Checksums and endpoint matching detect damage and attribute
traffic, but do not authenticate participants or pin addresses.

## Link measurement and retransmission

Each connection estimates its own round trip. Only first-transmission
acknowledgements contribute samples; retries use exponential backoff. One slow
link therefore does not set every connection's retry interval.

## Match timing

Every player reports process time and optional worst-local RTT as one record.
Reports expire after 512 frames. Missing initial RTT has that long to appear;
missing or stale established RTT selects `10/250` immediately. Stale process
data retains the last synchronized frame rate, and authorized removal clears
the player's report.

Compressed matches start at send rate 3 with nine frames of look-ahead. Worse
conditions apply immediately. Improvement needs three evaluations with 20%
headroom and a cooldown, moving one rung at a time. After eight changes, only
conservative increases remain.

A reduction activates after the old horizon drains, on a frame aligned to both
send periods. It switches to the new rate with temporary look-ahead, then drops
one new send period at each boundary. Replacement targets rebase this process.
A successor master inherits the target and change count, then clears
improvement evidence and starts a cooldown.

Latency margin remains 1×, 1.5×, 2×, or 3× RTT. Game speed keeps its existing
mapping, including speed zero as 60 FPS.

## Compatibility

Network events and recorded multiplayer commands include the timing reports.
All players must use the same OpenTS snapshot, and a recording should be played
by the snapshot that wrote it. There is no player-facing timing setting to
migrate.
