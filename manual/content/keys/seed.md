---
key: Seed
summary: The number the map generator's random sequence is started from.
see_also: [NumPlayers, Width, Height]
when_omitted:
  kind: value
  value: "-1"
  note: The request for a fresh number. The map generator dialog draws a replacement from 0 through 65535; a seed file read as the game starts hands `-1` to the random sequence as written.
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

:::caution[A seed file taken at the start of a scenario is not given a fresh number]
The replacement and the range check both belong to the dialog. A scenario that starts from a seed file goes through neither. A `Seed` of `-1` in such a file is handed to the random sequence as the number `-1`, and a figure above `65535` is handed over as written. Each still gives a map that is reproducible; it is simply not the map the same file would produce through the dialog. A seed file shipped with `-1` therefore serves up the same map every time it is started, since nothing on this path ever draws a fresh number.
:::
