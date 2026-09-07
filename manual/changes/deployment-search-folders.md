---
title: Search the folders a deployment keeps its files in
category: feature
release: 0.2.0
breaking: true
migration:
- Rename an `INI`, `MIX` or `Maps` directory beside the game whose files are not meant to be loaded, or ship an `OPENTS.INI` naming only the game's own directory as `SearchPaths=.`.
- Check a `Maps` directory in particular, since the maps it holds now appear in the game's own lists.
targets:
- type: format
  id: opents-ini
  effect: added
credit: [ZivDero]
---

A distribution can now sort its files into folders and name them in an `OPENTS.INI` beside its game data. With no such file the game searches `INI`, `MIX` and `Maps`, so a deployment sorted that way needs no configuration.

A wildcard search stopped at the first folder holding a match. Rules files, battle files, map packets, loose maps, and the map and movie archives are now gathered across every searched folder in a fixed order, so which copy of a repeated name is used no longer depends on the file system. The loose `PATCH.MIX` and `EXPAND??.MIX` archives are looked for in every searched folder rather than the game's own directory alone, and are still required to be loose.

The settings file, the hotkey file, the hall of fame and a saved random map were opened through the search before being written, so a copy shipped in a searched folder could be overwritten and the hotkey reset could delete it. The game now never writes into or deletes from a searched folder. The settings written when the intro is first shown are saved again; reopening the file for reading beforehand had left the save with a file it could not write through.
