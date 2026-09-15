---
key: FogOfWar
summary: Raises the fog of war over a campaign mission.
see_also: [FogRate, "system:map-visibility"]
when_omitted:
  kind: value
  value: "no"
  note: The campaign branch clears the flag before the map is read, so a mission that omits the key runs without fog whether or not the mission before it set one.
---

```ini title="map file"
[SpecialFlags]
FogOfWar=yes
```

The value is read into the scenario's own copy of the special options, the copy every part of the game reads when it asks whether fog is running. A scenario that finishes loading with the flag set has the fog raised over every cell not already under it. [Shroud, fog and the radar map](/systems/map-visibility/#the-fog-of-war) owns what the fog then does and how [`FogRate`](/keys/fograte/) brings it back over ground nothing is watching.

The entry is read from the map only in a single-player mission. A campaign clears the scenario's flag just before the map file is opened. The map's own assignment is then the only thing that can switch fog on, and nothing later in the loading order overwrites it. Every other game type skips the read and fills the scenario's copy from the game option before the map file is opened. Once the map has been read, it replaces the scenario's whole set of flags with the set the game options hold, so the entry reaches nothing there.
