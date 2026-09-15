---
title: Reconnect every scouting record to its house when loading
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

Every computer player scouting record is reconnected to the house it belongs to when a save is loaded. Loading previously reconnected the last record repeatedly and left the rest pointing at whatever occupied that memory in the saving session. A computer player resuming from a save could then read another house's scouting information as its own.
