---
title: Leave a human player's country and color to the game in SPAWN.INI
category: feature
release: 0.3.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
---

A human player's section in the client launch file, `SPAWN.INI`, may now leave out `Side` or `Color`, or give `-1`, and the game [draws the country and color](/formats/spawn-ini/#who-is-playing) as it does for a computer player. Such a file used to be refused.

Every country and color left to the game is now drawn before the map loads, the human players' first and then the computer players', in a game set up from the menu too. A drawn color leaves out every color another player gives; a computer player used to be able to draw the color a later computer player gave. With the same seed, computer players now draw different countries and colors than before.
