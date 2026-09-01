---
title: Hold the campaign difficulty setting to the settings it names
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

The campaign difficulty read from the settings file is now held to the three
difficulties the game has, rather than to five. A file edited by hand to name a
fourth or fifth could start a mission whose computer difficulty fell below the
easiest one and read the difficulty table from outside itself. A difficulty
chosen in the game is unaffected.
