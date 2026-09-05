---
title: Movies in multiplayer with a vote to skip
category: feature
release: 0.2.0
targets:
- type: system
  id: multiplayer-movies
  effect: added
- type: format
  id: spawn-ini
  effect: changed
- type: format
  id: vqa
  effect: changed
- type: key
  id: Win
  effect: changed
- type: key
  id: Lose
  effect: changed
- type: action
  id: TACTION_PLAY_MOVIE
  effect: changed
- type: action
  id: TACTION_PLAY_INGAME_MOVIE
  effect: changed
credit: [ZivDero, Rampastring]
---

A launch file that writes `PlayMoviesInMultiplayer=yes` now plays the scenario's movies in the
skirmish or the game against other machines it starts: the opening movies, the movies triggers
and scripts ask for, radar movies, and the `Win` or `Lose` movie after the score screen. In a
game against other machines a full-screen movie ends only once every player has pressed Escape,
and the machines keep exchanging packets while it plays, so a new packet joins the ones a match
accepts. The key was read before and changed nothing.

Rampastring is credited for the Vinifera feature this one follows.
