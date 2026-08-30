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
save, load, listing and deletion names that folder rather than searching for it, which is
where the launchers that browse saved games already look.

Saves made by earlier builds sit beside the game and are no longer listed; moving the `.SAV`
files into `Saved Games` restores them.

Following a load, the campaign difficulty pair now comes from the save rather than from the
menu's difficulty setting, so the next mission of a resumed campaign is played at the
difficulty it was saved at.
