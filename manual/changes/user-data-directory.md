---
title: Keep a player's own files in a directory named on the command line
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:user-directory
  effect: added
credit: [ZivDero]
---

`-USERDIR=<path>` tells the game where to keep what it writes. Every file the
game writes, creates or deletes goes there — the settings file, hotkeys, saved
games, the hall of fame, recordings, saved random maps, screenshots and the
files a multiplayer game downloads — and the directory is created when it is not
there yet. Saved games take a `Saved Games` folder of their own inside it.

It is read from before anywhere else, so a player's own copy of a file is the one
the game uses, whatever a deployment ships under the same name. Files a player
already has beside the executable are still read until their own copy exists, so
pointing an existing installation at a user directory carries them forward.

A file the game throws away is its own copy. Resetting the hotkeys discards the
player's and falls back to the ones a deployment shipped, rather than removing
what everyone shares.

Without the option every one of these files stays where it has always been,
beside the executable.
