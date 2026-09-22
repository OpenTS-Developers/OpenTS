---
title: Reconnect dialog
summary: "Opens when a network game has waited on a player's frames, shows who the game is waiting on, and drops the player furthest behind when the wait runs out unless the others vote someone out first."
category: multiplayer-networking
keys: []
related:
  - type: system
    id: out-of-sync-recovery
  - type: system
    id: network-packet-validation
---

A network game advances a frame only when every machine's events for that frame have arrived. When one machine's events are missing, the game halts and waits. The reconnect dialog opens once the wait has lasted about 7 seconds, or three times the slowest connection's average round trip if that is longer.

Before the first frame of a match, and again after a [multiplayer load](/systems/out-of-sync-recovery/#loading-a-saved-game), every machine waits until all the others report ready. At these starting barriers the dialog opens after about 14 seconds.

## What it shows

Every player in the match has a button with their name and a bar beside it. Your own bar stays full and green.

A machine that is waiting with you reports about once a second, so its bar stays green. If a machine reports during the wait and then stops, its bar turns yellow after about 4 seconds of silence and red after about 8, and it shrinks to a short stub at about 19 seconds.

The bar of a machine that has sent nothing since the game halted is not a reliable measure of its silence. It can show any color and length, and it can jump from green to a red stub at once. A red bar alone therefore does not prove which connection failed.

The line beneath the bars counts down to the end of the wait. The countdown starts when the game halts, not when the dialog opens.

The countdown lasts 2400 ticks by default, about 38 seconds. `ReconnectTimeout` in the [launch file](/formats/spawn-ini/#a-game-against-other-machines) sets it in ticks of 16 milliseconds, 62.5 to the second. The countdown shows 60 ticks as one second, so each second it shows lasts 0.96 real seconds. With the default it reads about 33 when the dialog opens in a running game.

In a running game the list beneath the countdown names the player furthest behind. It also explains how to vote a player out, or to leave with Cancel. At a starting barrier it says only that the game is waiting for connections.

## When the countdown runs out

In a running game, the player furthest behind is dropped, and the countdown starts over for anyone still missing.

After a drop the dialog stops updating for the same delay that first opened it, about 7 seconds by default. The countdown stays at or near zero during that delay, then resumes from about 33. Votes and Cancel take effect only once it resumes.

What becomes of the dropped player's base is set by the [departure policy](/systems/leaving-a-match/).

At a starting barrier the game gives up instead. It reports that the other system is not responding and ends the match on this machine.

## Kicking a player

Clicking another player's name proposes removing them. Clicking your own name only adds a reminder to use Cancel.

The proposal goes to every other machine. A machine whose dialog is already open counts it as your vote and reports it in its list. A machine whose dialog opens later never counts it. You have one vote against each player, so clicking the same name again sends nothing.

A player voted against by every other player in the match is dropped at once, without waiting for the countdown. If that player is you, you leave the match. Votes last only while the dialog is open; the next stall starts with none.

The [packet checks](/systems/network-packet-validation/) refuse a proposal from a machine that is not a player in the match.

## Leaving

Cancel leaves the match. The dialog closes by itself when the missing frames arrive. It also closes when a [multiplayer load](/systems/out-of-sync-recovery/#loading-a-saved-game) is requested, since the load resynchronizes every machine.
