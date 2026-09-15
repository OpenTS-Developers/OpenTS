---
key: Side
scope: multiplayer-settings
label: Preferred multiplayer side
see_also: ["Handle", "Color"]
when_omitted:
  kind: context-dependent
  note: The first house in the rules until a side has been chosen this run; the already-chosen side afterwards.
---

The game reads the side from the `[MultiPlayer]` section of `sun.ini` as the multiplayer menu is entered, and a new choice there is saved when you leave the LAN or skirmish dialog. The value names a house; the match ignores letter case and accepts either the house's section name or its display name. A number is read as a name too, so `Side=2` looks for a house called `2`, not for the second house. The game remembers the choice as that house's number among all loaded houses, not among the multiplayable ones only.

The LAN and skirmish dialogs preselect the entry of their side box with that country. The box offers only the houses marked as multiplayable. Each entry names its country, so the country chosen is the one played whatever position it holds in the rules.

:::caution[An unknown house name is invented rather than rejected]
A name that matches no loaded house creates a house under that name and stores its number, which no entry of the multiplayable list has. Both side boxes then open on their first entry. The invented house stays in the list for the rest of the run.
:::
