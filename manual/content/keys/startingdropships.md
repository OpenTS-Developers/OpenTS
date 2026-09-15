---
key: StartingDropships
summary: How many dropships the player fills before a campaign mission begins.
see_also: [AllowableUnits, AllowableUnitMaximums]
when_omitted:
  kind: value
  value: "0"
---

```ini title="map file"
[Basic]
StartingDropships=2
```

Any figure above zero opens the loadout screen after the briefing and before the action movie, with five slots to a dropship. The units chosen there fill the player house's three dropship loadouts in order. Zero skips the screen entirely, which is what nearly every mission does.

A reinforcement whose task force names `DSHP` and nothing else is built as a loaded dropship, and the same steps decide its cargo every time. The player's current loadout gates it: while the player has loaded nothing, the reinforcement is refused outright, so nothing goes wrong until the player has filled theirs. The player's loadout also sizes it: the dropship gets one member per unit the player is currently carrying. The sending house supplies the units: each member is read from the loadout of the house the reinforcement is sent for, at that house's current position, and nothing checks the result. Only the player's house is ever given a loadout. The player's own reinforcement therefore arrives carrying exactly what was chosen; one sent for any other house reads loadouts that hold nothing and hands every empty result to the game. Each delivery advances that house's position, and the mission's own triggers decide when each one lands.

[`AllowableUnits`](/keys/allowableunits/) decides what the screen offers.

:::danger[Asking for more than three reads past the end of fixed tables]
The screen picks its dropship picture and its slot positions out of tables with three entries each, and indexes them with the figure given, minus one, without checking the range. A figure of four or more reads past the end of both tables and hands the result to the file loader and the button layout. The chosen units are then stored into three per-house slots that a fourth dropship also overruns.
:::
