---
title: Out-of-sync recovery
summary: "What the players see when a network game goes out of sync, and how the master's choice to load a saved game, continue or quit is carried out on every machine."
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

Every machine checks the checksums the others report before it runs a frame's commands, so
each finds the divergence within a frame or two of the others and writes its
[report](/using/out-of-sync-reports/). It then halts the game and opens a dialog. The master
chooses between loading a saved game, continuing, and quitting; everyone else waits for that
choice, and can quit after ten seconds.

Both dialogs list the players. The host icon marks the master, and each player is shown as
`OK`, as `Out of sync` when their checksum disagreed with this machine's, or as `Left`. A chat
box under the list reaches everyone, and a line typed there is shown in the message list as
well, as [in-game chat](/systems/chat/) is.

No frames are exchanged while the dialog is open, so each machine sends the others a heartbeat
once a second instead. A player silent for twenty-five seconds is dropped, with the same
"connection lost" notice a player who stops answering during play gets, and a player who quits
is shown as such the moment their sign-off arrives.

## Continuing

Continue drops the players that are out of sync with this machine and plays on; their houses
pass to the computer, as they do for any player who leaves. The players on the other side of
the divergence do the same, so the match splits into games that carry on separately. The
players still in step with each other keep playing together, and a later divergence among them
opens the dialog again.

## Loading a saved game

In a client-launched match the master can instead pick one of the match's
[saved games](/formats/save-games/#loading-during-a-match). Every machine then shows a
five-second countdown and loads its own copy of the file the master named, keeps the seats it
had, and synchronizes again at the save's frame. A player who has left since the save was
written fights on under the computer. The same load can be started from the options menu in a
match that has not gone out of sync; the game keeps running under the list while the master
browses it.

A machine that cannot load the file leaves the match with the usual loading error, and the
others carry on without it.

## Quitting

Quit signs this machine off to every seat and ends the game here; for the others it is a
player leaving. On the waiting dialog the button enables after ten seconds, so nobody quits by
reflex.

## The master

Whoever is master decides, and every machine names the same one: the seat the
[launch file's host](/formats/spawn-ini/#the-host) announced, while that player holds a seat,
and otherwise the lowest seat still held. When the master leaves during the dialog, the next in
line takes over at once, and its waiting dialog becomes the decision dialog with the chat so far
kept.

A recording played back has nobody to decide with, so it keeps the plain "out of sync" box.
