---
title: Compatibility and save games
summary: How OpenTS lists and loads save games, and which save games a build accepts.
category: compatibility-migration
source_files:
  - README.md
  - code/loaddlg.cpp
  - code/saveload.cpp
  - code/savever.cpp
related:
  - type: using
    id: project-status
---

OpenTS uses the English Tiberian Sun 2.03 release as its inherited data and behavior baseline.

Every save file's header carries an internal version stamp. The load dialog lists a save only when its stamp matches the version the current build expects; files with any other stamp do not appear, and the multiplayer network save file is never listed. A save that reaches the engine without passing through the dialog is checked the same way and refused if its stamp does not match. A listed save that was not made in a campaign is marked with a `*` before its description.

OpenTS does not read save games written by the vanilla game or by a different
OpenTS release-cycle version, and no converter is provided. Development
snapshots within one cycle share a version stamp, so a save from an older
snapshot can appear in the list even though its stored layout may not match the
one the running snapshot reads. Finish or abandon a game in progress before replacing a development
snapshot.
