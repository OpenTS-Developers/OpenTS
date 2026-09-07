---
title: Write an out-of-sync report with bounded histories
category: feature
release: 0.2.0
targets:
- type: system
  id: developer-mode
  effect: changed
- type: command
  id: launch:desync-test
  effect: added
credit:
- ZivDero
- Rampastring
- dkeeton
---

An out-of-sync report was written as `SYNC<n>.TXT` in the working directory. It is now written into the `Debug` folder beside the executable, named for the local player and the frame, once per frame. It names every player whose checksum disagreed along with both sides' values, records the session identity and seed so two players' reports can be lined up, keys each object to its stable identifier, no longer draws from the shared random generator, and carries bounded, newest-first histories of the recent random draws, target assignments, mission orders, facing assignments, animation creations and events.

Divergence is detected before any of the frame's events run, so two players' reports describe the same point in the game. The new `-DESYNCTEST=<frame>` launch option corrupts the local checksum once so a report can be checked without waiting for a real divergence, and the message box reads "The game is out of sync." instead of "Reconnection Error!".

Rampastring is credited for the ts-patches state histories this follows and the Vinifera check that reads a frame's checksums before its events run, dkeeton for the expanded ts-patches sync file this takes the FPU control word from.
