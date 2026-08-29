---
title: Search the folders a deployment keeps its files in
category: feature
release: 0.2.0
breaking: true
migration:
- Rename an `INI` or `MIX` directory beside the game whose files are not meant to be loaded, or ship an `OPENTS.INI` naming only the game's own directory as `SearchPaths=.`.
targets:
- type: format
  id: opents-ini
  effect: added
- type: command
  id: launch:cd-path
  effect: changed
credit: [ZivDero]
---

A distribution can sort its files into folders and name them in an `OPENTS.INI`
beside its game data. With no such file the game searches `INI` and `MIX`,
so a deployment can sort its files and ship no configuration at all.

Wildcard searches now cover every folder the game searches rather than stopping
at the first one holding a match. Rules files, battle files, map packets, loose
maps, and the map and movie archives are all found across the folders, which
also means the folders a `-CD` argument adds now contribute to them. Names are
gathered in a fixed order, so which copy of a repeated name is used no longer
depends on the order a file system reported it in.

The loose `PATCH.MIX` and `EXPAND??.MIX` archives are now looked for in every
searched folder instead of the game's own directory alone. They are still
required to be loose files, so an expansion archive cannot be hidden inside
another archive.

Files the game writes are never written into a searched folder, and a file it
deletes is never one of theirs. The settings file, the hotkey file, the hall of
fame and a saved random map were opened through the search before being written,
so a copy a deployment had shipped in one of these folders could be overwritten,
and the hotkey reset could delete it. Both now settle on the game's own copy
before writing.

The settings written when the intro is first shown are saved again. The file was
reopened for reading beforehand, which left the save with a file it could not
write through.
