---
title: Multiplayer movies
summary: "Plays a scenario's movies in a skirmish or a game against other machines when the launch file asks for them, and lets the machines of a match cut a movie short only by agreement."
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

A skirmish or a game against other machines shows no movies. A launch file that writes
`PlayMoviesInMultiplayer=yes` changes that for the game it starts: the movies a campaign
mission would show play in that game as well. [Client launch file](/formats/spawn-ini/) owns
the key; this page owns what then plays and how a movie ends.

## What plays

- The scenario's [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and
  [`Action`](/keys/action/#scope-scenarios) movies as it starts.
- A movie a trigger or a team script asks for,
  through [Play Movie...](/mapping/actions/taction-play-movie/) or
  [Play movie...](/mapping/missions/tmission-play-movie/), and a movie played into the radar
  pane through [Play Ingame Movie...](/mapping/actions/taction-play-ingame-movie/).
- The [`Win`](/keys/win/) or [`Lose`](/keys/lose/) movie, after the score screen.

Each movie is still held to the other conditions [VQA video](/formats/vqa/) lists. Every
machine must hold the movie and every machine's file must carry the key: a machine that passes
over a movie the others watch waits for them in the meantime, and the waits a game against
other machines keeps for a machine that has gone quiet apply to that wait.

## Skipping a movie together

In a game against other machines a full-screen movie stops only once every player and observer
in the match has pressed Escape. The first press on any machine puts two lines over the movie on
every machine: who wants the movie skipped, then either how many have agreed and a prompt to
press Escape, or, on a machine that has pressed it, that it is waiting for the others. A player
who leaves the match is no longer waited for. The movie plays on while the votes are collected
and ends on its own if they never all arrive.

The machines keep talking while a movie plays. Each tells the others once a second which movie
it is watching and whether it wants it skipped, and again the moment its player presses Escape,
so a vote cast before another machine has reached the movie still counts and the match's traffic
never falls silent. Nothing else of the match runs meanwhile; the frames resume once the movie
ends on every machine.

A movie in a game against other machines keeps playing when its window loses focus, so a machine
whose player has switched away does not hold the others back. In a campaign or a skirmish the
movie pauses without focus, as it always has.

## Where Escape ends the movie alone

Escape ends the movie at once, with no vote, in a skirmish, in a campaign, and for the `Win` and
`Lose` movies of any game: those play after the score screen, when nothing keeps the machines in
step any more.
