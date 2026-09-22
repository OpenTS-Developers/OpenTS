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

A player who leaves a client-launched match now has their base destroyed. That is what `AutoSurrender`, in the launch file's `[Settings]` section, asks for, and what the file says unless it writes `No`. A match arranged from the game's own menu still hands the base over. Where the computer takes a seat over it keeps the player's name rather than renaming the house to itself. The radar list, chat and the score screen still say who held it.

`ConnTimeout` and `ReconnectTimeout` in the launch file's `[Settings]` section are read at last. They set how long this machine waits on another stalled on the loading screen, and on one gone quiet during play, in ticks of 16 milliseconds, 62.5 to the second. They default to the waits the game already kept, and each machine keeps its own.

Closing the window during a match, or pressing Alt and F4, resigns the way the options menu's abort does rather than being ignored. `ContinueWithoutHumans` is no longer read, so a line an existing launch file has does nothing. A match ends when the last person playing leaves, unless it is seated entirely by observers.
