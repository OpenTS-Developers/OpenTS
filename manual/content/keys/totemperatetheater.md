---
key: ToTemperateTheater
summary: Parsed tile-set number that the engine stores and never reads.
no_effect: true
see_also: [ToSnowTheater]
when_omitted:
  kind: value
  value: "-1"
---

The key names the tile set that takes this one's place when a map is converted to the temperate theater. Nothing in OpenTS performs that conversion. The value is stored on the first tile of each entry in the set and read back by nothing. No draw uses it, and the multiplayer synchronization checksum never visits the isometric tile types.

The lettered alternates of a tile do not receive the value at all, so even the stored copy covers only part of the set.
