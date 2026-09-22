---
command_id: ScreenCapture
---

Saves the current frame as `SCRN0000.png` in a `Screenshots` folder. The folder is in the game's own directory, or in the [user directory](/using/game-data/) when one is set. The number is the lowest one not yet used in the folder, so a capture never overwrites an earlier one. A capture creates the folder if it is missing, even when it was deleted while the game was running.

The picture is the whole frame at the game's render resolution, whatever the window size, and does not include the mouse pointer. If the capture cannot be written, any partial file is deleted and the failure is recorded in the [debug log](/using/debug-logging/).
