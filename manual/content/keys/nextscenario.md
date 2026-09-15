---
key: NextScenario
summary: The mission the campaign advances to when this one skips the map selection screen.
see_also: [AltNextScenario, SkipMapSelect, OneTimeOnly]
when_omitted:
  kind: context-dependent
  note: The read falls back to the stored name itself. The scenario object empties the name once when the session starts, so the session's first mission falls back to an empty name and any later mission to the name the previous mission's file wrote.
---

```ini title="map file"
[Basic]
SkipMapSelect=yes
NextScenario=Maps/Missions/GDI2A.MAP
```

The name is used only when [`SkipMapSelect`](/keys/skipmapselect/) is set and global variable 1 is clear; with that variable set, [`AltNextScenario`](/keys/altnextscenario/) is taken instead. Nothing else reads it, so a mission that leaves the map selection screen in place can set any value here without effect.

The advance runs through the campaign's map selection data rather than loading the file directly. The name is compared, ignoring case, against the scenario of each stage the mission's current stage offers as a next step. A match moves the campaign to that stage. The path written therefore has to be the same path the progression data records, and [campaign progression](/systems/campaign-progression/#campaigns-and-stages) covers where that data lives.

:::caution[An unreachable name sends the player back to the start of the campaign]
A name matching no offered stage shows an error box and drops the player at the campaign's own first mission rather than at the map selection screen. [Choosing the next mission](/systems/campaign-progression/#choosing-the-next-mission) covers why, and which nearby failures reload the mission just won instead.
:::
