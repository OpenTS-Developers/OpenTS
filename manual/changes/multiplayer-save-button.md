---
title: Offer the save in a network game's options
category: feature
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

The options dialog of a game against other machines now offers Save Game. The game has
long known how to make the save — one press submits the synchronized command and every
machine writes its own copy at the same frame — but the network dialogs never carried the
button that asks for it. It greys out once a player has left the match, as the saving
rules always said.
