---
title: Leaving a match
summary: "Ends a player's part in a network game, whether they quit or fall silent, and decides whether their base is destroyed or handed to the computer."
category: multiplayer-networking
keys: []
related:
  - type: format
    id: spawn-ini
  - type: system
    id: reconnect-dialog
  - type: system
    id: out-of-sync-recovery
  - type: system
    id: observers
---

A player leaves a network game by quitting, by going silent for too long, or by being dropped when the master [continues](/systems/out-of-sync-recovery/#continuing) an out-of-sync game. Every machine removes the player's seat on the same frame. The other players see a message that says whether the player left or lost the connection. No message names a player dropped from an out-of-sync game.

A player quits with any of these:

- Abort Mission in the options menu, then Abort.
- The window's close button or Alt+F4, described [below](#the-close-button).
- Quit in the [out-of-sync dialog](/systems/out-of-sync-recovery/).

Surrender, in the same dialog as Abort, does not leave the match. It destroys the player's base and keeps them in the game.

The [launch file](/formats/spawn-ini/#a-game-against-other-machines) sets how long the other machines wait for a silent player:

- On the loading screen, `ConnTimeout` is how long loading may go without progress. When it runs out, every player who has not finished loading is dropped.
- During play, `ReconnectTimeout` is the wait that the [reconnect dialog](/systems/reconnect-dialog/) counts down. When it runs out, the player furthest behind is dropped.

## What becomes of their base

`AutoSurrender` in the [launch file](/formats/spawn-ini/#the-options-every-house-plays-under) decides what happens to the base of a player who leaves:

- Omitted, or written as `Yes`, it destroys the player's buildings and units where they stand, after the short [`SavourDelay`](/keys/savourdelay/) countdown.
- Written as `No`, it hands the base to the computer, which keeps playing it.

Every machine's launch file must give `AutoSurrender` the same value. A match set up from the game's own menu always hands the base to the computer.

A player who leaves keeps their name on the seat, even when the computer takes it over. The radar list, chat and the score screen still show who held it.

An [observer](/systems/observers/) has no base, so an observer's departure leaves nothing to destroy or hand over.

## When the match ends

A destroyed base leaves its house with nothing, so that house is defeated as soon as the destruction happens, and the other players are told. A base the computer took over is defeated only when it loses in play.

The match ends when no person is left playing it, meaning every player has left or been defeated. The game checks this each time a house is defeated. When the last person still playing leaves and the computer takes over their base, no house is defeated, so the match continues until one is.

A match in which every seat is an [observer](/systems/observers/) never has a person playing, so that rule does not apply. It continues until only one side remains.

## The close button

During a game, the window's close button and Alt+F4 quit the same way as Abort in the options menu. In a network game the other players are told the player left, and the departure then follows the rules above. Outside a game, and while a recorded game plays back, the close button does nothing.
