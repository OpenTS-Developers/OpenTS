---
command_id: QuickLoad
---

Loads the quick save for the kind of game being played: `QUICKSAVE.SAV` in a campaign or `QUICKSAVE_SKIRMISH.SAV` in a skirmish. If that file is missing or was written by another version, the message list shows `No quick save to load.` and nothing else happens.

Otherwise the load runs at the end of the frame, and play resumes in the restored game. If the restore fails partway, the game shows `Error loading game!` and leaves the player in the options menu, as a failed load from the load dialog does.

The command does nothing in any of these cases:

- the game is not a campaign or a skirmish;
- a scripted sequence has locked input;
- the game is already being won or lost.
