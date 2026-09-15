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

Every save file's header holds an internal version stamp, which is the project version the game prints in the corner of its title screen. The load dialog lists a save only when its stamp matches the version the current build expects. A file with any other stamp does not appear, and the multiplayer network save file is never listed. A save that reaches the engine without passing through the dialog is checked the same way and refused if its stamp does not match. The [`QuickLoad`](/commands/quickload/) command is one such path; it reads the quick save directly and reports that there is no quick save when its stamp does not match.

OpenTS does not read save games written by the vanilla game or by a different OpenTS release-cycle version, and no converter is provided. Development snapshots within one cycle share a version stamp, so a save from an older snapshot can appear in the list. Its stored layout may not match the one the running snapshot reads. Finish or abandon a game in progress before replacing a development snapshot.
