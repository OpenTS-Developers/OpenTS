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

A build lists and loads only saves written by the same OpenTS version. Every save records a version stamp: the major, minor, and patch numbers of the version shown in the corner of the title screen, and a serial that development builds raise when the save layout or the simulation changes. Each release resets the serial to 0. A prerelease label such as `-beta1` and the build details in parentheses after the number are not part of the stamp.

The load dialog skips any save whose stamp differs from the running build's. A save opened without the dialog is checked the same way and refused. For example, [`QuickLoad`](/commands/quickload/) shows that there is no quick save to load when the quick save has another stamp. [Save games](/formats/save-games/#what-is-checked) lists the other paths that check the stamp.

The campaign and skirmish load dialog lists only `.SAV` files. Saves from a game against other machines are listed separately, during a match; see [Loading during a match](/formats/save-games/#loading-during-a-match).

OpenTS cannot load saves from the original game. There is no converter for those saves or for saves from another OpenTS version, so finish or abandon a game in progress before you move to another version.

Two builds with the same stamp can still store a save differently, and such a save is listed but can fail to load:

- Development snapshots of one version share its stamp until the serial is raised, and a layout change made without raising it leaves two snapshots with one stamp. Finish or abandon a game in progress before replacing a snapshot.
- The Win32 and x64 builds share the stamp. Load a save with the platform that wrote it, and play a network game with every player on the same platform; a mixed network game can go out of sync.
