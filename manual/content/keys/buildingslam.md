---
key: BuildingSlam
summary: The sound played as a structure the local player has placed settles onto the map.
see_also: [BuildingDrop, CrumbleSound, "system:production"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
BuildingSlam=PLACE2 ; a sound ID registered in SOUND.INI
```

The sound is played once a finished structure has been placed and its factory closed out, and only when the placing house is the local player's. Another player putting a structure down is silent on this side of the connection. Walls and firestorm wall segments run the same branch, so laying a wall makes the same sound as raising a building.

It is played without a position, so it comes through at full volume wherever the view happens to be sitting.
