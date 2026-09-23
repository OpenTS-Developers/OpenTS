---
title: Out-of-sync recovery
summary: "What the players see when a network game goes out of sync, and how the master's choice to load a saved game or continue is carried out on every machine."
category: multiplayer-networking
keys: []
related:
  - type: using
    id: out-of-sync-reports
  - type: format
    id: save-games
  - type: format
    id: spawn-ini
  - type: system
    id: network-packet-validation
  - type: system
    id: chat
---

## When the game goes out of sync

Before a machine runs a frame's commands, it compares its own checksum with the checksums the other machines reported. Every machine therefore detects a divergence at the same frame. Each machine then writes its [out-of-sync report](/using/out-of-sync-reports/), halts the game, and opens the out-of-sync dialog.

The [master](#the-master) gets a decision dialog with three buttons: Load Game, Continue and Quit. Every other player gets a waiting dialog whose only button is Quit.

Both dialogs list the players. The host icon marks the master, and each player has one of these statuses:

- `OK`: in step with this machine.
- `Desynced`: their checksum disagreed with this machine's.
- `Quit`: they left while the dialog was open. The status appears as soon as their sign-off arrives or this machine drops them for silence.

A chat box under the list sends to every player. A line typed there also appears in the on-screen message list, as [in-game chat](/systems/chat/) does.

The game sends no frames while the dialog is open, so each machine sends the others a heartbeat once a second. Each machine drops a player after twenty-five seconds with no heartbeat from them. The count starts when that machine's dialog opens and restarts with each heartbeat. The dialog then lists that player as `Quit`, and the on-screen message list gets the same "connection lost" line as for a player who stops answering during play.

A recording played back has nobody to decide with. It shows the plain "The game is out of sync." box, with Continue and Stop.

## Continuing

When the master presses Continue, every machine drops the players who are out of sync with it and plays on. The bases of the dropped players go the way of any player who [leaves the match](/systems/leaving-a-match/#what-becomes-of-their-base).

The machines out of sync with the master drop the master in the same way, so the match splits into separate games that carry on independently. If the players in one of those games diverge later, the dialog opens again.

## Loading a saved game

The master can instead press Load Game and pick one of the match's [saved games](/formats/save-games/#loading-during-a-match). The button is grayed out when the master has no saved game from this match. The game stays halted for everyone while the master browses the list.

Once the master picks a save, every machine shows a five-second countdown in its dialog and then loads the [save with the same number](/formats/save-games/#numbered-multiplayer-saves) from its own Saved Games folder. The master's Load Game and Continue buttons are grayed out during the countdown. The computer plays the house of anyone who has left since the save was written.

A machine that cannot load the save shows the usual loading error and leaves the match. The other machines carry on without it.

The master can start the same load from the options menu in a match that has not gone out of sync. There the game keeps running while the master browses the list. If the game goes out of sync while that load is counting down, each machine still writes its report, but no dialog opens and the load goes ahead.

## Quitting

Quit tells every other machine that this player is leaving, and ends the game on this machine. The other machines treat it as a player [leaving the match](/systems/leaving-a-match/). The master can quit at once. A waiting player's Quit button becomes available ten seconds after the dialog opens.

## The master

The master decides for everyone. In a match started from the menu, or from launch files that name no host, the master is the lowest seat. In a match started from launch files, the [host](/formats/spawn-ini/#the-host) is the master when it holds the lowest seat.

The master keeps the role until they leave the match. The lowest seat still held then becomes the master on every machine, as the [master handoff](/systems/network-synchronization/#master-handoff) describes. After a [multiplayer load](#loading-a-saved-game), the host named by the launch files is the master if it is still playing, whichever seat it holds.

When the master leaves while the dialog is open, nobody takes over. The other players cannot continue or load a saved game, and can only quit.
