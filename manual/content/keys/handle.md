---
key: Handle
summary: The player name remembered between runs and offered back by the multiplayer dialogs.
see_also: ["Color", "Side"]
when_omitted:
  kind: value
  value: "[NONAME]"
  note: The English placeholder text; a localized build supplies its own.
---

The name is read as the multiplayer menu is entered and fills the name field of the LAN and skirmish dialogs. Edits there are saved to the `[MultiPlayer]` section of `sun.ini` when you leave the dialog. On the LAN it is also the name the game is advertised under when the player hosts. The stored name holds up to 63 characters, and the LAN dialog limits typing to sixteen.
