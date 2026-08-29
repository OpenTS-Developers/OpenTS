---
title: Launch a game from a client's launch file
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:spawn
  effect: added
- type: format
  id: spawn-ini
  effect: added
credit: [ZivDero]
---

Starting the game with `-SPAWN` now plays the skirmish that `SPAWN.INI` describes. The
startup movies and the main menu are both skipped, the map list the menu would scan is not
read, and the game exits when the match ends rather than returning to a menu the client
never meant to show. The settings file a client manages is no longer written back to while
a launch is in progress.

The file names the options every house plays under, who is playing, each seat's country,
color, difficulty and start position, and the alliances between them. A file describing a
campaign mission, a saved game, or a game against other machines is refused with the reason
shown; those launches arrive separately. Anything the file asks for that the game cannot
honor is listed on the launch file's own page.

The node the player and lobby lists are made of now initializes itself rather than starting
as whatever the heap last held, and carries the start position, difficulty and alliances a
seat asked for. No packet the game sends and no save it writes carries that node, so
neither format changes, and a game set up from the menu is assembled exactly as before.
