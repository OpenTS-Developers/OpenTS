---
format_id: keyboard-ini
title: KEYBOARD.INI
summary: Maps registered command names to integer keyboard identifiers.
kind: file
filenames:
  - KEYBOARD.INI
source_files:
  - code/init.cpp
  - code/keyboard.h
  - code/options.cpp
---

`[Hotkey]` keys are the exact registered command names. Each value is an integer keyboard identifier: the key's Windows virtual-key code, plus 256 for Shift, 512 for Control and 1024 for Alt.

```ini title="KEYBOARD.INI"
[Hotkey]
SelectView=577      ; A with Control
ToggleRepair=338    ; R with Shift
ScatterObject=88    ; X
```

After the file loads, OpenTS clears the current hotkey table and adds entries whose command name is registered and whose keyboard identifier is not zero. Unknown command names and zero values are ignored, and a name has to match the registered spelling exactly, including its case.

The table is cleared only once the file has been read, so a file that is missing or that cannot be parsed leaves the bindings already in force rather than emptying them. OpenTS looks the file up through the ordinary file layer, so a loose `KEYBOARD.INI` in the game directory stands in for an archived one. The keyboard dialog writes it back as a loose file from the bindings the player accepted; cancelling the dialog drops its edits, and its reset control deletes the file and rebuilds the table without it.

A command that carries a forced binding is bound again once the file has been processed, taking that key back from whatever the file gave it. The file's own binding for that command is left alone, so it can end up answering to two keys.

Two commands take a key by default instead: [`ChatToAll`](/commands/chattoall/) takes Enter and [`ChatToAllies`](/commands/chattoallies/) takes Backspace, each only when the file binds neither that command nor that key. A file that binds the command to another key, or gives the key to another command, is left as written.

:::caution[Two commands on one key are both kept]
Nothing rejects an entry for a key that is already bound. Both entries are added to the table, and the key runs one of the two commands without regard to the order they appear in the file.
:::
