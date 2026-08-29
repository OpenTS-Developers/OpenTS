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

`-USERDIR=<path>` tells the game where to keep what it writes: the settings
file, hotkeys, saved games, the hall of fame, recordings, saved random maps and
the files a multiplayer game downloads. The directory is created when it is not
there yet, and it is searched ahead of the game's own folders, so a map a player
received is found before a copy the deployment shipped.

Settings and saved games a player already has beside the executable are still
read from there until the game writes them to the new directory, so pointing an
existing installation at one carries them forward.

Without the option every one of these files stays where it has always been,
beside the executable.
