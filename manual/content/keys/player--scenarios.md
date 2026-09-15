---
key: Player
scope: scenarios
label: Commanded house
see_also: [SpeechSide, NextScenario]
when_omitted:
  kind: computed
  note: The first HouseType the rules registered, because an absent or empty value resolves to "no house" and that result is replaced with the first one.
---

```ini title="map file"
[Basic]
Player=GDI
```

The value names a HouseType and settles which house the player commands for the rest of a campaign mission. Everything created afterwards is measured against that house: ownership, allegiance, buildable lists, and the money the mission starts with. The campaign's map selection screen reads its progression list from the same house. A multiplayer or skirmish game assigns houses from the lobby and never reaches this read.

The lookup accepts either the house's ID or its full name and ignores case.

:::danger[A house the scenario does not contain leaves the player pointer null]
The houses a mission contains are the first HouseTypes the rules registered, as many of them as the map's `[Houses]` list has entries. When the named house is not among them, the search for it returns nothing and the very next statement writes through the empty result. A name that matches no registered house at all is worse still. Rather than being rejected, it registers a fresh HouseType, which no scenario can ever contain. A typo here is enough to end the load in a null write.
:::
