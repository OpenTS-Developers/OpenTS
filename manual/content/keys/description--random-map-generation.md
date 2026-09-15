---
key: Description
scope: random-map-generation
label: Saved map name
see_also: [Seed, NumPlayers]
when_omitted:
  kind: context-dependent
  note: The read falls back on the name the game itself gives a random map, which is taken from the language files, so the stored text follows the language the game is running in.
---

The text is what the load and delete dialogs list the seed under and what the multiplayer lobby shows as the scenario name once the map is generated. It is kept in a 128-character buffer and anything longer is cut to fit. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="MyMap.SED"
[RandomMap]
Description=Four player temperate map
```

The dialogs build their lists by reading the assignment out of each candidate file directly, and that read has no fallback. A file whose `Description` is missing or empty is listed as unusable rather than under a substitute name, and cannot be picked. The generator's own scratch file is passed over and never listed at all.

The text takes no part in what is built. It is deliberately blanked before the settings are digested into the identifier a lobby matches maps by, so renaming a seed does not make it a different map.
