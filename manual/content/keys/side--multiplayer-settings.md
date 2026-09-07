---
key: Side
scope: multiplayer-settings
label: Preferred multiplayer side
see_also: ["Handle", "Color"]
when_omitted:
  kind: unchanged
  note: The side already chosen this run, which is the first house in the rules when the game starts.
---

The value is a house identifier, matched without regard to letter case against both the section names and the display names of the loaded houses. What gets stored is that house's position in the list, and the LAN and skirmish dialogs preselect the entry of their side box carrying that country; the box offers only the houses marked as multiplayable, and each entry carries its country, so the country chosen is the one played whatever position it holds in the rules.

:::caution[An unknown house name is invented rather than rejected]
A name that matches no loaded house creates a house under that name and stores its position, which no entry of the multiplayable list carries. Both side boxes then open on their first entry. The invented house stays in the list for the rest of the run.
:::
