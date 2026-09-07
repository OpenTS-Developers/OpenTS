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

`-USERDIR=<path>` tells the game where to keep what it writes. The settings file, hotkeys, saved games, the hall of fame, recordings, saved random maps, screenshots and the files a multiplayer game downloads all go there, in a directory created when it is not there yet, with saved games in a `Saved Games` folder of their own. Without the option every one of these files stays beside the executable, as before.

The directory is read from before anywhere else, so a player's own copy of a file is the one the game uses whatever a deployment ships under the same name, and files already beside the executable are still read until that copy exists. A file the game throws away is its own copy: resetting the hotkeys discards the player's and falls back to the ones a deployment shipped, rather than removing what everyone shares.
