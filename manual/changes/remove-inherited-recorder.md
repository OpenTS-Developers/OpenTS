---
title: Remove the inherited game recording and playback
category: feature
release: 0.3.0
breaking: true
migration:
- Remove `-XX` and `-XY` from the command line of a Debug build; the game no longer starts with either.
targets:
- type: command
  id: launch:record
  effect: removed
- type: command
  id: launch:playback
  effect: removed
- type: command
  id: launch:attract
  effect: removed
- type: key
  id: Frame
  effect: removed
- type: key
  id: Type
  scope: multiplayer-settings
  effect: removed
- type: key
  id: Coord
  effect: removed
- type: key
  id: Target
  effect: removed
- type: key
  id: Cell
  effect: removed
- type: key
  id: PrintCRC
  effect: removed
credit: [ZivDero]
---

The Debug-build options `-XX` and `-XY`, which recorded each game to `RECORD.BIN` and played it back, are removed; a recording never played past its first frame. Both are now unknown `-X` letters, so the game exits before starting, as a Release build already did. `-ATTRACT`, which only affected that playback, is now ignored.

The `[SyncBug]` settings in `sun.ini` are no longer read: `Frame`, `Type`, `Coord`, `Target`, `Cell` and `PrintCRC`. Only `PrintCRC` had an effect, and only during the removed playback.
