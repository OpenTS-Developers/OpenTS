---
key: Time
summary: The hour a generated map is fought at, as a position from 0 through 3.
see_also: [Biome, UseTransitions]
when_omitted:
  kind: value
  value: "1"
  note: The fallback of `1` is the afternoon, so the map takes the afternoon's light and no floodlight ring is planted.
---

The positions are `0` morning, `1` afternoon, `2` dusk and `3` night. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Time=3
```

The hour settles two things. It picks the map's ambient light: three quarters at morning and at dusk, full at afternoon, half at night. A snow theater then cuts that light to three quarters again, so the darkest map the generator builds is a snow map at night. The hour also picks how many floodlights are planted in a ring around each player's start point: none in the morning or the afternoon, two at dusk, four at night. A ring is only planted where every light in it can legally stand. Twenty-one ring angles are tried before the start point is left dark.

With [`UseTransitions`](/keys/usetransitions/) set the hour also names the settings file loaded into the map, and every start point is ringed with four lights whatever the hour.

:::danger[A figure outside 0 through 3 reads past two tables]
The map generator dialog offers the four hours only, and the settings taken off it are held to that range, but a seed file read as the game starts goes through neither. A figure written by hand indexes both the four-entry light-level table, which is read before anything is built, and the four-entry floodlight-count table. The map's ambient light is then set from whatever the first read returns. The number of floodlights each start point is ringed with comes from whatever the second returns.
:::
