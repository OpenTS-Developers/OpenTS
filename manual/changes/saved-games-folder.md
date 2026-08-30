---
title: Keep saved games in a folder of their own
category: feature
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

Saved games now live in a `Saved Games` folder — beside the game, or inside the user data
directory when one is named — created the first time the game asks for a saved game. Every
save, load, listing and deletion names that folder, and it is deliberately not one of the
folders the game searches: a saved game is written, so it is named rather than found. That
is where the launchers which browse saved games already look, which is what makes resuming
one from a launch file possible.

Saves made by earlier builds sit beside the game and are no longer listed; moving the `.SAV`
files into `Saved Games` restores them.

Following a load, the campaign difficulty pair now comes from the save rather than from the
menu's difficulty setting, so the next mission of a resumed campaign is played at the
difficulty the campaign was saved at. Before, it silently took whatever the setting happened
to say at the time.
