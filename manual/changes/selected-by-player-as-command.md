---
title: Offer the selected-by-player trigger event on every computer
category: fix
release: 0.3.0
targets:
- type: event
  id: TEVENT_SELECTED
  effect: changed
credit:
- ZivDero
---

In a multiplayer game, selecting a tagged object offered the "selected by player" trigger event on the selecting player's computer only, so a trigger that changed the game put the match out of sync. The selection now reaches every computer as a command, and the event is offered on all of them when it runs, one look-ahead after the selection. In a campaign mission or skirmish, the event is now offered after the logic of the frame in which the player selected the object instead of before it.
