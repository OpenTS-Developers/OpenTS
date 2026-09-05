---
title: Reconnect dialog
summary: "Opens when a network game has waited on a player's frames, shows how long each player has been silent, and drops the slowest player when the wait runs out unless the others vote someone out first."
category: multiplayer-networking
keys: []
related:
  - type: system
    id: out-of-sync-recovery
  - type: system
    id: network-packet-validation
---

A network game advances a frame only when every machine's events for it have arrived. When one machine's are missing, the game halts and waits. The reconnect dialog opens once that wait has lasted seven seconds, or three times the measured round trip when that is longer. At the barrier that starts a match, and at the one that follows a [multiplayer load](/systems/out-of-sync-recovery/#loading-a-saved-game), it opens after fifteen seconds instead.

## What it shows

Every player has a button carrying their name and a bar beside it. Your own bar stays full and green. Another player's bar measures the time since their last frame message: it is green for four seconds, yellow until eight, red after that, and shrinks to a stub at twenty, all measures of silence rather than fractions of the wait. The line beneath counts down the wait itself, forty seconds unless a [launch file](/formats/spawn-ini/#a-game-against-other-machines) sets `ReconnectTimeout` otherwise, and the list under it explains the situation and offers the choices below.

When the countdown reaches zero in a running game, the player furthest behind is dropped and the wait starts over for anyone still missing. What becomes of their base is the [departure policy](/systems/leaving-a-match/). At a starting barrier the game gives up instead and reports that the other system is not responding.

## Kicking a player

Clicking a player's name proposes a kick. The proposal goes to every machine, each records it as your vote and reports it in the list, and a player has one vote against any given target. A player voted against by every other player is dropped at once; if that player is you, you leave the match. Clicking your own name only posts a reminder. The [packet checks](/systems/network-packet-validation/) refuse a proposal from outside the seated roster.

## Leaving

Cancel leaves the match. The dialog closes by itself when the missing frames arrive, and when a [multiplayer load](/systems/out-of-sync-recovery/#loading-a-saved-game) is requested, since the load resynchronizes every machine.
