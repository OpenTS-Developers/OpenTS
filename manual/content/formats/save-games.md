---
format_id: save-games
title: Save games
summary: Stores versioned OpenTS game state in `.SAV` compound-document files.
kind: binary
extensions:
  - .SAV
role: persistence
source_files:
  - code/event.cpp
  - code/goptions.cpp
  - code/init.cpp
  - code/loaddlg.cpp
  - code/mainloop.cpp
  - code/netdlg.cpp
  - code/saveload.cpp
  - code/savestream.cpp
  - code/savever.cpp
  - code/abstract.cpp
  - code/objtype.cpp
  - code/unittype.cpp
---

The save dialog creates `.SAV` files. Each file is an OLE compound document: the listing details live in the document's own property set, and the game state goes into a single `CONTENTS` stream that is compressed as it is written.

## Where the files are

Saved games keep to a `Saved Games` folder of their own, beside the game or inside the [user directory](/using/game-data/) when one is named, created the first time the game asks for a saved game. Every save, load, listing and deletion names that folder outright: unlike the files the game reads, a saved game is never looked for anywhere else. A client that browses saved games therefore finds them in one place, whichever layout the game was installed in.

The dialog names a new save `SAVE` followed by four hexadecimal digits, drawing again until it finds a name no existing file answers to; saving over a listed game reuses that game's name. A multiplayer save is written under one fixed name instead and is never offered in the list. The random map generator keeps its saved settings in the same folder, under names of its own; the map a host generates for a match is not one of them, and stays with the game's files so that it can travel to the other machines.

## When the file is written

A campaign or skirmish save requested through the save dialog is written immediately while that dialog has the scenario paused. A multiplayer click instead submits a synchronized `SAVEGAME` command. When that command executes, each peer copies one pending filename and description; duplicate commands before the frame ends share that one request. The file is written only after the command queue has finished and the end-of-frame deletion pass has retired every object already marked for removal.

Once a connection is destroyed or a synchronized `REMOVEPLAYER` command executes, multiplayer saving is disabled for the rest of that match and any pending request is cancelled. The options dialog disables its Save button in that state. Restarting the mission does not restore the button or accept another request; selecting and starting a new game does.

## What the file holds

The property set carries the description shown in the list, the player's name and house, the campaign and scenario numbers, the game type, three timestamps, the name of the program that wrote the save, and two version stamps — the save format's own version and the build version of the game that wrote it.

The `CONTENTS` stream is a fixed sequence of records — the scenario, the environment, the rules, the map, the loose global values, and every list of type definitions and runtime objects — written and read back in the same order. Each list stores its own length ahead of its members, and each member writes out the members its class declares, in the order that class lists them. What a save holds is therefore a field-by-field record of each object rather than a copy of the bytes it occupied in memory. Type definitions travel with the save, so a save carries the rules types it was made with rather than looking them up again on load. Artwork does not travel with it: once a restored type's members have been read, its shape and voxel pointers are released and fetched from the archives again, so a save loaded against a changed set of files gets the current artwork. One piece does not come back. A UnitType drawn from shapes is given a [voxel turret](/formats/vxl-hva/) when the rules are read, by a routine no restore calls; the restore takes the ordinary voxel path instead, which releases that turret along with the body model it could not find. Its voxel barrel is fetched back, and the barrel is what the shape path draws.

## What is checked

The project-version stamp decides whether a file is offered at all, and only
the running version's stamp is accepted. The load dialog reads the property set
of every `.SAV` in the saved-games folder and skips every file stamped by
anything else, including the Tiberian Sun release and another OpenTS
release-cycle version. A save that reaches the engine without passing through
the dialog, as a network save or one resumed from a
[launch file](/formats/spawn-ini/) does, is checked the same way and refused.
Development snapshots within one cycle share the stamp, and their save layouts
may still differ. A listed save that was not made in a campaign is marked with a
leading `*`.

Beyond that stamp and the add-on the scenario declares, nothing about a save is measured against the game it is being loaded into. A save made under one set of rules and loaded under another is not detected, and the type definitions stored in the file are simply restored over the ones the rules built.

Reading `CONTENTS` clears the scenario before it starts and gives up at the first record it cannot restore. A load that stops there fails, rather than carrying on into a scenario that was cleared and never refilled.
