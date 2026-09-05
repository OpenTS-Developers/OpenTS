---
format_id: save-games
title: Save games
summary: Stores versioned OpenTS game state in `.SAV` compound-document files.
kind: binary
extensions:
  - .SAV
role: persistence
source_files:
  - code/autosave.cpp
  - code/conquer.cpp
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
  - code/ambient.cpp
  - code/voc.cpp
---

The save dialog creates `.SAV` files. Each file is an OLE compound document: the listing details live in the document's own property set, and the game state goes into a single `CONTENTS` stream that is compressed as it is written.

## Where the files are

Saved games keep to a `Saved Games` folder of their own, beside the game or inside the [user directory](/using/game-data/) when one is named, created the first time the game asks for a saved game. Every save, load, listing and deletion names that folder outright: unlike the files the game reads, a saved game is never looked for anywhere else. A client that browses saved games therefore finds them in one place, whichever layout the game was installed in.

The dialog names a new save `SAVE` followed by four hexadecimal digits, drawing again until it finds a name no existing file answers to; saving over a listed game reuses that game's name. A multiplayer save is written under one fixed name instead and is never offered in the list. The random map generator keeps its saved settings in the same folder, under names of its own; the map a host generates for a match is not one of them, and stays with the game's files so that it can travel to the other machines.

## When the file is written

A campaign or skirmish save requested through the save dialog is written immediately while that dialog has the scenario paused. A multiplayer click instead submits a synchronized `SAVEGAME` command. When that command executes, each peer copies one pending filename and description; duplicate commands before the frame ends share that one request. The file is written only after the command queue has finished and the end-of-frame deletion pass has retired every object already marked for removal.

Once a connection is destroyed or a synchronized `REMOVEPLAYER` command executes, multiplayer saving is disabled for the rest of that match and any pending request is cancelled. The options dialog disables its Save button in that state. Restarting the mission does not restore the button or accept another request; selecting and starting a new game does.

A save that cannot be written, whether the player asked for it or the game did, says so in the message list.

## Automatic saves

The game also saves on its own at a fixed interval of frames: a game started from the menu at the interval [`AutoSaveInterval`](/keys/autosaveinterval/) names, and a [client-launched](/formats/spawn-ini/#automatic-saves) game at the interval its launch file names. When the interval runs out, `Auto-saving...` is posted to the message list and the save is written at the next frame boundary, after the notice has been drawn, through the same request the synchronized multiplayer save uses. The interval starts over from any completed save, whoever asked for it, and from a load or a mission restart, so a resumed or restarted game waits a full interval before its first.

A campaign writes `AUTOSAVE1.SAV` through `AUTOSAVE5.SAV` in turn and then starts over, and a skirmish writes `AUTOSAVE_SKIRMISH1.SAV` through `AUTOSAVE_SKIRMISH5.SAV` the same way; the two rings turn independently. Each is described as `Auto-Save`, its slot number and the scenario's description, so a listing tells it apart from a save the player named. Every save records the slot that follows the last one written, in both rings, and a loaded game continues from what its save records. The rings keep their positions for as long as the game runs, so a new game started from the menu carries on where the last automatic save left off rather than overwriting it, and a client-launched game starts where its launch file says.

A game against other machines saves automatically only when a launch file set the interval, because every machine must write the same frame and a match arranged from the menu leaves each machine with settings of its own. Each machine then writes `SAVEGAME.NET`, described as `Multiplayer Game (Auto-Save)`, through the pending request and without the saving box a manual save shows, so a client watching the folder can number the file as it numbers a manual save. Once multiplayer saving is disabled for the match, automatic saves stop with it.

## Quick saves

The [`QuickSave`](/commands/quicksave/) command writes a campaign to `QUICKSAVE.SAV` and a skirmish to `QUICKSAVE_SKIRMISH.SAV`, replacing the previous file of that kind, so a skirmish never writes over a campaign. The save is written at the frame boundary after the key was pressed, once the frame has retired its dead objects, behind the saving box a menu save shows; the message list then reports `Mission Saved` or that the game could not be saved. Each file is described as `Quick Save` and the scenario's description, and the load dialog lists it like any other save. A quick save starts the automatic-save interval over like any completed save.

[`QuickLoad`](/commands/quickload/) restores the file for the kind of game being played. It first reads the file's property set and refuses, with a line in the message list, when the file is missing or carries another version's stamp; otherwise the load runs when the frame ends, in the place the options menu runs, and the mission clock resumes in the restored game. A load that fails partway through the restore shows the same error box as the load dialog and leaves the player in the options menu.

Both commands are refused in a game against other machines, during playback, while a scripted sequence has locked input, and once the game is being won or lost. Both arrive unbound.

## What the file holds

The property set carries the description shown in the list, the player's name and house, the campaign and scenario numbers, the game type, three timestamps, the name of the program that wrote the save, and two version stamps — the save format's own version and the build version of the game that wrote it.

The `CONTENTS` stream is a fixed sequence of records — the scenario, the environment, the rules, the map, the loose global values, and every list of type definitions and runtime objects — written and read back in the same order. Among the loose values are the looping sounds left at waypoints by [Play Sound Effect At](/mapping/actions/taction-play-sound-at/) and the sounds attached to objects, each as the sound and its place or object; the playing sound itself is not saved and starts again on the first sound tick after the load. Each list stores its own length ahead of its members, and each member writes out the members its class declares, in the order that class lists them. What a save holds is therefore a field-by-field record of each object rather than a copy of the bytes it occupied in memory. Type definitions travel with the save, so a save carries the rules types it was made with rather than looking them up again on load. Artwork does not travel with it: once a restored type's members have been read, its shape and voxel pointers are released and fetched from the archives again, so a save loaded against a changed set of files gets the current artwork. One piece does not come back. A UnitType drawn from shapes is given a [voxel turret](/formats/vxl-hva/) when the rules are read, by a routine no restore calls; the restore takes the ordinary voxel path instead, which releases that turret along with the body model it could not find. Its voxel barrel is fetched back, and the barrel is what the shape path draws.

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
