---
key: EndOfGame
summary: Whether winning this mission closes the campaign with its ending movie and credits.
see_also: [OneTimeOnly, SkipMapSelect, PreMapSelect]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
EndOfGame=yes
```

The setting is read after the score screen and both after-mission movies. When it is set, the mission is treated as the last of its campaign: the campaign's own closing movie plays, the credits roll, and the game returns to the main menu without advancing to another map.

The closing movie belongs to the campaign rather than to the mission, and it is played only when the mission was started as part of one. A mission that belongs to no campaign still ends the session and still rolls the credits, but shows no movie first.

[`OneTimeOnly`](/keys/onetimeonly/) is tested first and takes precedence, so a mission with both ends the session with neither the movie nor the credits.
