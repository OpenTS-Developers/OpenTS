---
title: macOS application bundle uses home-directory data and saves
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:data-directory
  effect: changed
- type: command
  id: launch:user-directory
  effect: changed
- type: system
  id: ui-files
  effect: changed
credit:
- fjlaubscher
---

A macOS game launched from an application bundle reads Tiberian Sun data from `Games/OpenTS` in the home directory, writes settings and saves to `Library/Application Support/siliconsun`, and reads its `ui` files from `Contents/Resources/ui` inside the bundle. `-DATADIR` and `-USERDIR` still name those directories when they are passed. A launch that is not from an application bundle keeps its previous directories.
