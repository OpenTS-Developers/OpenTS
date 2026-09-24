---
title: Keep Goto Radar Event off the factory a unit leaves
category: fix
release: 0.2.0
targets:
- type: command
  id: CenterOnRadarEvent
  effect: changed
credit: [ZivDero, dkeeton, Rampastring]
---

Goto Radar Event no longer jumps to a factory because a unit or aircraft left it. Every finished unit used to move the place the command jumps to, replacing the attack or loss the player wanted to find. In a multiplayer game another player's finished unit moved it as well, to that player's factory.

dkeeton is credited for the ts-patches change this follows, and Rampastring for the same change in Vinifera.
