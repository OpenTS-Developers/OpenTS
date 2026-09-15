---
key: CheckHeap
summary: Parsed heap check switch that the engine never acts on.
no_effect: true
see_also: ["Frame", "PrintCRC"]
when_omitted:
  kind: value
  value: "0"
---

Despite its name, no heap consistency check runs alongside the game. The engine reads the value from the `[MultiPlayer]` section of `sun.ini` as the multiplayer menu is entered. A non-zero value sets a global flag at the top of every game frame, and nothing reads that flag back.
