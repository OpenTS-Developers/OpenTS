---
title: Message your team, or the other observers, and choose who a line reaches
category: feature
release: 0.2.0
breaking: true
migration:
- Remove `-MESSAGES` from any shortcut. The option is gone; a chat line is accepted only from a seat of the match.
targets:
- type: system
  id: chat
  effect: added
- type: command
  id: ChatToAll
  effect: added
- type: command
  id: ChatToAllies
  effect: added
- type: command
  id: fixed:multiplayer-message
  effect: changed
- type: command
  id: launch:messages
  effect: removed
- type: format
  id: keyboard-ini
  effect: changed
- type: system
  id: observers
  effect: changed
credit: [ZivDero, dkeeton, CCHyper]
---

In-game chat now has a line to the team and, for an observer, a line to the other observers. Both come beside the lines to everyone and to one player that chat always had. Backspace starts a team line and Enter a line to everyone unless the keyboard file gives those keys or commands to something else. A line is delivered and shown only where its kind allows. A team line reaches the houses the sender is allied with, an observers line reaches the other observers, and a line from outside the match is dropped. The `-MESSAGES` switch that let such lines through is gone.

dkeeton is credited for the ts-patches team and observer chat this follows, and CCHyper for the Vinifera routing and echo.
