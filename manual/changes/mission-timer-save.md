---
title: Keep the elapsed mission time across a save game
category: fix
release: 0.1.0
targets: []
credit: [ZivDero]
---

The elapsed mission time now survives a save and load. It reset to zero whenever a game was loaded. The time reported at the end of a mission therefore counted from the last load, not from the start of the mission.
