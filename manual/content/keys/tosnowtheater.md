---
key: ToSnowTheater
summary: Parsed tile-set number that the engine stores and never reads.
no_effect: true
see_also: [ToTemperateTheater]
when_omitted:
  kind: value
  value: "-1"
---

The name promises the tile set that takes this one's place when a map is converted to the snow theater. The value is stored on the first tile of each entry in the set. Nothing reads it back: no conversion, no draw, and not the multiplayer synchronization checksum, which never visits the isometric tile types.

The lettered alternates of a tile do not receive the value at all, so even the stored copy covers only part of the set.
