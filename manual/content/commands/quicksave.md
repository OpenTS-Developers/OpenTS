---
command_id: QuickSave
---

Saves a campaign to `QUICKSAVE.SAV` and a skirmish to `QUICKSAVE_SKIRMISH.SAV`, replacing the previous quick save of that kind. The file is written at the end of the frame, behind the saving box a menu save shows. The message list then shows `Game saved.` or `The game could not be saved.` The file goes in the [saved games folder](/formats/save-games/#where-the-files-are), and [Save games](/formats/save-games/#quick-saves) says how the load dialog lists it.

The command does nothing in any of these cases:

- the game is not a campaign or a skirmish;
- a scripted sequence has locked input;
- the game is already being won or lost.
