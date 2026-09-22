---
title: Save screenshots as PNG in a folder of their own
category: feature
release: 0.2.0
targets:
- type: command
  id: ScreenCapture
  effect: changed
credit:
- ZivDero
- Rampastring
- dkeeton
---

Screen captures are now PNG files in a `Screenshots` folder, beside the game or in the user data directory when one is set. They used to be PCX files among the game's own files. Captures from earlier builds are not moved.

A capture is now the frame as the game renders it. It used to be scaled and offset at some window sizes.

Rampastring and dkeeton are credited for the ts-patches patches that first moved captures into a folder and saved them as PNG.
