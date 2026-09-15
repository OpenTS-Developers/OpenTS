---
title: Multiplayer movies
summary: "Plays a scenario's movies in a skirmish or a game against other machines when the launch file asks for them, and cuts a full-screen movie short in a match only when every machine agrees."
category: multiplayer-networking
keys: []
related:
  - type: format
    id: spawn-ini
  - type: format
    id: vqa
  - type: system
    id: network-packet-validation
  - type: command
    id: fixed:skip-vqa
---

A skirmish or a game against other machines shows no movies unless its launch file asks for them. A launch file that writes `PlayMoviesInMultiplayer=yes` turns them on for the game it starts: the movies a campaign mission would show play there as well. [Client launch file](/formats/spawn-ini/) owns the key; this page owns what then plays and how a movie ends.

## What plays

- The scenario's [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and [`Action`](/keys/action/#scope-scenarios) movies as it starts.
- A movie a trigger or a team script asks for, through [Play Movie...](/mapping/actions/taction-play-movie/) or [Play movie...](/mapping/missions/tmission-play-movie/), and a movie played into the radar pane through [Play Ingame Movie...](/mapping/actions/taction-play-ingame-movie/).
- The [`Win`](/keys/win/) or [`Lose`](/keys/lose/) movie, after the score screen.

Each movie is still held to the other conditions [VQA video](/formats/vqa/) lists. Every machine's launch file must set the key. A machine that lacks the movie file itself passes over the movie the others watch and waits for them in the meantime; that wait follows the [reconnect rules](/systems/reconnect-dialog/) the match applies to a machine that has gone quiet.

## Skipping a movie together

In a game against other machines a full-screen movie stops only once every player and observer in the match has pressed Escape. The first press on any machine puts two lines over the movie on every machine. One names who wants the movie skipped. The other holds the vote tally, and it either asks for an Escape press or reports that this machine is waiting for the others. A player who leaves the match is no longer waited for. The movie plays on while the votes are collected and ends on its own if they never all arrive.

The machines keep talking while a movie plays. Each machine tells the others once a second which movie it is watching and whether it wants it skipped, and again the moment its player presses Escape. That repeated report is why a vote cast before another machine has reached the movie still counts, and why the match's traffic never falls silent. Nothing else of the match runs meanwhile; the frames resume once the movie ends on every machine.

A movie in a game against other machines keeps playing when its window loses focus, so a machine whose player has switched away does not hold the others back. In a campaign or a skirmish the movie pauses without focus, as it always has.

## Where Escape ends the movie alone

Escape ends the movie at once, with no vote, in a skirmish, in a campaign, and for the `Win` and `Lose` movies of any game. In a game against other machines those two play after the score screen, when nothing keeps the machines in step any more.
