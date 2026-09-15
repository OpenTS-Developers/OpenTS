---
key: Scenario
scope: campaign
label: Campaign opening mission
see_also: ["CD", "Description", "FinalMovie", "NextScenario"]
when_omitted:
  kind: value
  value: ""
  note: An empty name, which starts nothing.
---

The value is the map file the campaign begins with, and it is folded to upper case as it is stored. The stored name is opened as written, so the `.MAP` extension is part of the value and the shipped campaigns write paths such as `Maps/Missions/GDI1A.MAP`. Choosing the campaign from the mission list starts that file as its first mission. A campaign that names no file fails to start and drops back to the menu. After a mission is won, the next one is picked on the map selection screen, or named by the mission's [`NextScenario`](/keys/nextscenario/) when it skips that screen. That screen is the operation map drawn from `MAPSEL.INI` for the player's house, where the player clicks one of the next-mission targets the won mission offers; [choosing the next mission](/systems/campaign-progression/#choosing-the-next-mission) owns it. The sequence, and the state it carries over, belongs to [campaign progression](/systems/campaign-progression/).

The same stored name is searched for `GDI` whenever the campaign's disc number is above `1`, which is how a Firestorm campaign's loading screen decides which side's artwork to draw.
