---
title: Read and show text as UTF-8
category: feature
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit:
- ZivDero
---

Game text, INI files, typed input, player names and chat are UTF-8. The shipped fonts draw
the Western European letters and symbols they hold and show `?` for anything else. An INI
file that is not valid UTF-8 is read as Windows-1252; player names hold 64 bytes and chat
lines 224, and the UTF-8 code page needs Windows 10 version 1903 or newer.
