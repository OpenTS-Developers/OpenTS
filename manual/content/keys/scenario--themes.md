---
key: Scenario
scope: themes
label: Score unlock mission
see_also: [Normal, Side]
when_omitted:
  kind: value
  value: "0"
---

The score is withheld from the playlist until the campaign has reached the mission number written here, compared against the number the running scenario holds. The test applies to campaign games only. A skirmish or multiplayer session skips it, so no score is withheld by this key there.

```ini title="theme.ini"
[VALVES1B]
Name=Valves
Length=3.27
Scenario=1
```

Withheld means withheld from the automatic playlist and from the sound options track list alike. A score can still be started outright by name whatever this is set to.
