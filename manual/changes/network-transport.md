---
title: Play network games over UDP instead of IPX
category: feature
release: 0.1.0
breaking: true
migration:
- Remove `-SOCKET` and `-DESTNET` from any shortcut. Both options are gone, and `-DESTNET` has no counterpart because it addressed a bridge only IPX had.
- Delete `Socket`, `NetCard` and `DestNet` from the `[Network]` section of `sun.ini`, or leave them to be ignored.
- Check that every player can reach the others on one of the networks their machine is attached to. All players use UDP port 1234, and a router between two of them normally stops the broadcast that finds a game.
targets:
- type: key
  id: Socket
  effect: removed
- type: key
  id: NetCard
  effect: removed
- type: key
  id: DestNet
  effect: removed
- type: command
  id: launch:destination-network
  effect: removed
- type: command
  id: launch:socket
  effect: removed
credit: [ZivDero, tomsons26]
---

Network games are played over UDP. Tiberian Sun reached the other machines on a
local network through IPX, which Windows no longer carries, so network play had
stopped working entirely; it now uses the same UDP that internet play always
used. Games are found by broadcasting on every network the machine is attached
to, and each player answering is recorded at the address the answer came from.
Internet, skirmish, and campaign play are unaffected.

One machine can host only one game at a time. The network options screen is
gone, along with the adapter, socket, and destination network it set.

Network messages, save games, and recordings changed shape, so different OpenTS
release-cycle versions refuse one another.
