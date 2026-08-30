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

Saved games now live in a `Saved Games` folder, beside the game or inside the user data
directory when one is named. Every save, load, listing and deletion names that folder, which
is where launchers that browse saved games look.

Saves made by earlier builds sit beside the game and are no longer listed; moving the `.SAV`
files into `Saved Games` restores them.

After a load, the campaign difficulty now comes from the save rather than the menu setting.
