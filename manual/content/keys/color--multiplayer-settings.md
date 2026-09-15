---
key: Color
scope: multiplayer-settings
label: Preferred multiplayer color
see_also: ["Handle", "Side"]
when_omitted:
  kind: value
  value: "0"
---

The number is a position in the eight-entry multiplayer color list, which runs gold, red, blue, green, orange, sky blue, purple, pink. It preselects the color box in the LAN and skirmish dialogs. A new choice there is saved to the `[MultiPlayer]` section of `sun.ini` when you leave the dialog.

The game host can also set the position. Whenever the options it broadcasts name a different color beside the player's handle, the player is moved to that color, and it becomes the remembered choice.

A position outside the list leaves the color box with nothing selected rather than being corrected.
