---
key: RandomMap
summary: Builds the scenario's map with the random map generator instead of reading it from the file.
see_also: [Seed, NumPlayers, FreeRadar]
when_omitted:
  kind: value
  value: "no"
---

With `RandomMap=yes` in a scenario file's `[Basic]` section, the [random map generator](/systems/map-generation/) builds the map. The generator takes its settings from the file's `[RandomMap]` section, which uses the same keys as a [map seed file](/formats/map-seed/). A setting the section leaves out takes its default, and a value outside a setting's allowed range is moved to the nearest limit.

```ini title="map file"
[Basic]
Name=Random desert
RandomMap=yes
FreeRadar=yes

[RandomMap]
Width=2
Height=2
NumPlayers=2
Biome=3         ; desert

[MCV]           ; a rules section, applied to this match
Speed=5
```

The file's other sections are still read. Rules sections such as `[MCV]` above change the rules for the match, and `[Basic]` settings such as [`FreeRadar`](/keys/freeradar/) work as on an ordinary map. The generator writes its own `[Map]` size, theater and `Fill`, and its own `[Lighting]` `Ambient`, `Red`, `Green`, `Blue`, `Ground` and `Level`, replacing any values the file gives. It also places the start points. If the other sections cannot be read, the map fails to load as an ordinary map would.

The map is built from the match's seed, not from the `[RandomMap]` section's [`Seed`](/keys/seed/). A match started from a [launch file](/formats/spawn-ini/) uses the seed that file names: every machine builds the same map, and a different seed builds a different one. A `.SED` seed file needs no `RandomMap` key: it is always generated, from its own `Seed`, so it builds the same map every time.
