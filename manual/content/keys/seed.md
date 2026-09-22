---
key: Seed
summary: The number the map generator's random sequence is started from.
see_also: [NumPlayers, Width, Height]
when_omitted:
  kind: value
  value: "-1"
  note: A seed file without `Seed` builds the map `Seed=0` gives. `-1` becomes `0` whether the file is loaded into the map generator dialog or played as a scenario.
---

Every random choice the generator makes comes from one sequence started from this number, so the same settings and the same seed give back the same map, cell for cell. Two maps differing only in seed are laid out entirely differently. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Seed=12345
Width=1
Height=1
NumPlayers=4
```

`-1` is the request for a fresh number: the map generator dialog replaces it with a draw from `0` through `65535` as it opens, and the preview and generate buttons replace it again if it has somehow survived. The dialog's randomize button draws from the same range, and the settings taken off the dialog are held to it.

:::caution[A seed file never gets a fresh seed]
Whether a seed file is loaded into the map generator dialog or played as a scenario, a negative `Seed`, `-1` included, becomes `0`, and a value above `65535` becomes `65535`. A seed file shipped with `Seed=-1` therefore builds the same map as `Seed=0` every time.
:::
