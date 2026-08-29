---
title: Hold the campaign difficulty setting to the settings it names
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

The campaign difficulty read from the settings file is now held to the three
difficulties the game has, rather than to five. The game has offered three
since its sliders were built that way, so a setting chosen in the game is
unaffected; a file edited by hand to name a fourth or fifth could previously
start a mission whose computer difficulty fell below the easiest one and read
the difficulty table from outside itself.
