---
command_id: QuickLoad
---

Loads the quick save for the kind of game being played: `QUICKSAVE.SAV` in a campaign or `QUICKSAVE_SKIRMISH.SAV` in a skirmish. A missing file, or one written by another version, puts `No quick save to load.` in the message list and changes nothing else. Otherwise the load runs when the frame ends, and play resumes in the restored game. A file that fails partway through the restore shows the loading error. The player is then left in the options menu, as a failed load from the load dialog does. The command does nothing in a game against other machines, during a scripted sequence that has locked input, or once the game is being won or lost.
