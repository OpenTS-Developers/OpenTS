---
title: Read the game's data from a directory named on the command line
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:data-directory
  effect: added
credit: [ZivDero]
---

`-DATADIR=<path>` tells the game where its data is kept. Everything the game
reads is looked for there as well as beside the executable, and the deployment
file describing how that directory is sorted is read from it.

The game never writes to the directory, so one copy of the data can be shared
between the people using a machine, and can be installed somewhere they are not
allowed to write to. A directory that is not there stops startup with a message
rather than leaving the game to fail later over a missing file.
