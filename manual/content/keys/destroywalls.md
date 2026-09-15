---
key: DestroyWalls
summary: Whether a computer house in this difficulty slot scores walls as targets while it scans.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "yes"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores yes rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own flag, and an object reads the one for [the difficulty slot its house holds](/systems/difficulty/#from-the-setting-to-a-slot). `no` is the second gate on [wall targeting](/systems/target-selection/#picking-the-winner), immediately after the gate that restricts the whole behavior to computer houses. It cannot change anything for a house under player control. It stops walls being weighed while an object scans for something to shoot at, but it does not stop an object attacking a wall it is put onto by an order or a script.

Unlike the multipliers in the same section, this flag is not folded into a figure the house keeps. It is read from the section each time a cell is weighed, so a house that changes slot changes behavior at once.
