---
key: Side
scope: multiplayer-settings
label: Preferred multiplayer side
see_also: ["Handle", "Color"]
when_omitted:
  kind: unchanged
  note: The side already chosen this run, which is the first house in the rules when the game starts.
---

The value is a house identifier, matched without regard to letter case against both the section names and the display names of the loaded houses. What gets stored is that house's position in the list, and the LAN and skirmish dialogs use the position to preselect their side box, which offers only the houses marked as multiplayable.

:::caution[An unknown house name is invented rather than rejected]
A name that matches no loaded house creates a house under that name and stores its position, which lies past the end of the multiplayable list. The LAN and lobby side boxes then open with nothing selected, while the skirmish dialog clamps the position to its second entry. The invented house stays in the list for the rest of the run.
:::
