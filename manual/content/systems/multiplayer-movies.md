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

`PlayMoviesInMultiplayer=yes` in the [client launch file](/formats/spawn-ini/) turns movies on for the skirmish or game against other machines that the file starts. Without it, neither kind of game shows a movie. With it, the game shows the scenario movies listed below.

## What plays

- The scenario's [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and [`Action`](/keys/action/#scope-scenarios) movies, as the match starts.
- A full-screen movie from a trigger's [Play Movie...](/mapping/actions/taction-play-movie/) action or a team's [Play movie...](/mapping/missions/tmission-play-movie/) mission.
- A movie in the radar pane from a trigger's [Play Ingame Movie...](/mapping/actions/taction-play-ingame-movie/) action.
- The [`Win`](/keys/win/) or [`Lose`](/keys/lose/) movie, after the score screen.

Each movie must also pass the other tests [VQA video](/formats/vqa/) lists.

In a game against other machines, every machine's launch file must set the key, and every machine needs every movie file. A machine whose file leaves the key off, or that lacks the movie's file, does not play that movie.

The match stops while any machine is watching a movie, and resumes when the movie has ended everywhere. A machine that did not play the movie goes on into the match and waits there for the machines still watching, as it would for a lagging player, so its [reconnect dialog](/systems/reconnect-dialog/) opens. If the movie lasts longer than the reconnect countdown (about 38 seconds by default), that machine drops a player who is still watching it.

## Skipping a movie together

In a game against other machines, a full-screen movie stops early only after every player and observer in the match has pressed Escape. Until then it keeps playing, and it ends normally if the votes never all arrive. A player who quits the match is no longer counted. A player whose connection drops while the movie plays is still counted, so the movie then runs to its end. A machine that did not play the movie never votes, so the machines watching it cannot cut it short.

The first Escape press on any machine shows two lines over the movie on every machine watching it:

- The first line says who wants to skip. It names the player when one other player has pressed Escape, says "You" when only this machine has, and gives a count when more than one other player has.
- The second line shows the votes so far out of the total. It asks this machine's player to press Escape to agree, or, once they have, says the machine is waiting for the others.

An Escape pressed before another machine has reached the movie still counts there once it does.

In a game against other machines, a full-screen movie keeps playing when the game window loses focus, so a player who switches away does not hold up the others. In a campaign or a skirmish the movie pauses while the window is out of focus.

## Where Escape ends the movie alone

Escape ends a movie at once, without a vote, in a skirmish, in a campaign, and during the `Win` and `Lose` movies of a game against other machines. Those two play after the score screen, when the machines no longer keep in step.
