---
title: Keep network games apart from earlier OpenTS versions
category: internal
release: 0.3.0
targets:
- type: system
  id: network-synchronization
  effect: changed
breaking: true
migration:
- Update every player in a network game to this version.
credit:
- ZivDero
---

This version ignores the network games of earlier OpenTS versions, and they ignore its games. A LAN game hosted on the other version is not listed, and the machines of a game a client starts across the two versions never connect.
