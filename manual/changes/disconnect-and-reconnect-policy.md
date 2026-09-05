---
title: Decide what happens when a player leaves a match
category: feature
release: 0.2.0
targets:
- type: system
  id: leaving-a-match
  effect: added
- type: system
  id: reconnect-dialog
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
breaking: true
migration:
- A client or mod that wants a departed player's base handed to the computer must write `AutoSurrender=No` in the launch file. Without it a client-launched match now destroys the base.
credit:
- ZivDero
- Rampastring
---

A player who leaves a client-launched match now has their base destroyed, which is what
`AutoSurrender` asks for and what a launch file says unless it writes `No`. Where the computer
does take the seat over, it keeps the player's name rather than renaming the house to itself,
so the radar list, chat and the match statistics still say who held it. A match arranged from
the game's own menu still hands the base over.

`ConnTimeout` and `ReconnectTimeout` are read at last, in frames of which there are sixty to
the second: how long this machine waits on another that has stopped making progress on the
loading screen, and on one that has gone quiet during play. They default to the waits the game
already kept, and each machine keeps its own.

Closing the window during a match, or pressing Alt and F4, resigns the way the options menu's
abort does rather than being ignored.

`ContinueWithoutHumans` is no longer read. A match ends when the last person playing leaves,
except one seated entirely by observers, which is the rule it already followed.
