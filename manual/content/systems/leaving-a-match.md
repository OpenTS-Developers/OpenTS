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

A player leaves either on purpose or by falling silent until a wait runs out. The deliberate routes are the options menu, the close button and the out-of-sync dialog's Quit. A silent player waits out [spawn.ini's](/formats/spawn-ini/#a-game-against-other-machines) `ConnTimeout` on the loading screen. During play the wait is `ReconnectTimeout`, the one the [reconnect dialog](/systems/reconnect-dialog/) counts down. The other players are told which it was, a departure or a lost connection. Both go through the same synchronized removal, so every machine drops the seat on the same frame.

## What becomes of their base

The [launch file](/formats/spawn-ini/#the-options-every-house-plays-under) settles it with `AutoSurrender`. Left as it comes, the departed player's buildings and units are destroyed where they stand. Written as `No`, the computer takes the base over and plays it out. A match arranged from the menu hands it over, no file asking otherwise. A tournament game destroys it whatever the file says.

The seat keeps the player's name either way, in the radar list, in chat and in the statistics the match reports, so a later report still says who was where.

## When the match ends

A match ends once no person is left playing it. The exception is a match in which every seat is [watching](/systems/observers/) rather than playing. With nobody left to lose it, that match runs until one side remains.

A destroyed base leaves its house with nothing, so it is beaten at once and the other players are told. A base the computer took over is beaten only when it loses.

## The close button

Closing the window during a match, or pressing Alt and F4, resigns the way the options menu's abort does. The other machines are told, the statistics are reported, and the game ends itself rather than the window being torn out from under it. Outside a match the button does nothing.
