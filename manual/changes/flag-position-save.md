---
title: Reconnect a house's flag position when loading
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

A house's flag position is reconnected when a save is loaded. The record of where the flag rests, or of the unit carrying it, was written to the save file but never translated back into a reference to the loaded object. It kept pointing into the saving session instead. Games whose rules place flags, such as capture the flag, are the ones affected.
