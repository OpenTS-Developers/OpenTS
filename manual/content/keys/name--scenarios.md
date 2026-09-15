---
key: Name
scope: scenarios
label: Scenario title
see_also: [RequiredAddOn]
when_omitted:
  kind: value
  value: "<none>"
  note: The read substitutes the literal text "<none>" rather than keeping the title the previous mission established.
---

```ini title="map file"
[Basic]
Name=GDI 10A: Destroy the Prototype Facility (A)
```

The title is held in a 44-byte field, so it is cut off after 43 characters. Within the running game, the title becomes the description recorded with saves of the mission. The save offered from the in-game options screen starts from it, and the quicksave and automatic saves build their descriptions around it.

In a campaign the map does not have the last word. Once the map has been read, `MISSION.INI` is opened (`MISSION1.INI` under the Firestorm expansion) and searched for a section named exactly as the scenario file was named when the mission was started. A `Name=` there replaces the map's. The shipped campaigns do not reach it: their scenarios are named by path, such as `Maps/Missions/GDI1A.MAP`, while the shipped mission file's sections are bare filenames such as `[GDI1A.MAP]`, so the map's own title stands.
