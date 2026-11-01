---
title: Setup screen for the freeware game data
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:data-directory
  effect: changed
breaking: false
credit:
- fjlaubscher
---

A game with no `TIBSUN.MIX` shows a setup screen that downloads the freeware files, or copies them from a folder. Tiberian Sun game data and music are included. The campaign cinematics of each game are optional, and so are Firestorm's game data and music. Choosing the Firestorm cinematics includes Firestorm, and clearing Firestorm clears its cinematics. The files are written into the data directory. A finished setup offers to restart the game or quit, and the next launch starts the game with the installed content. An interactive launch creates a missing default data directory. A `-DATADIR` path that is not there is still refused. A directory that already holds `TIBSUN.MIX` is left unchanged. Headless and spawned runs do not show the screen.
