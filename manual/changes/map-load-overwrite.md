---
title: Stop a load from discarding restored radar and display state
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

The radar and the display layers come back from a save as they were left. Loading used to restore them correctly, then overwrite them with a copy of the map taken at save time. The lists that track what the radar draws and which objects sit in each display layer ended up describing the saving session rather than the game just loaded.
