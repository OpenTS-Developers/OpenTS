---
key: BuildingDrop
summary: The sound played as a deploying construction vehicle becomes a structure.
see_also: [BuildingSlam, DeploySound, UndeploysInto]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
BuildingDrop=PLACE2 ; a sound ID registered in SOUND.INI
```

The sound is played once at the deploying vehicle's own position, at the point in the deploy where the vehicle's group, crew and orders have been handed across to the structure and before its target is assigned. It is played only when the new structure's house is player-controlled: in a campaign that is any house flagged human or player-controlled, and in a skirmish or multiplayer game only the local player's. A structure deployed by any other house is silent and is instead marked for rebuilding and repair.

This is not the placement sound. A structure completed from the sidebar and put down by the player plays [`BuildingSlam`](/keys/buildingslam/) instead.
