---
key: UseTransitions
summary: Lets a generated map's lighting move with the hour instead of holding the hour it was built at.
see_also: [Time, UseIonStorms, Biome]
when_omitted:
  kind: value
  value: "no"
  note: The fallback of `no` holds the map's lighting at the hour it was built at and rings each start point with the hour's own floodlight count.
---

With the flag set, generation loads the settings file belonging to the hour named by [`Time`](/keys/time/): `MORNING.INI`, `DAY.INI`, `DUSK.INI` or `NIGHT.INI`. It takes three things from that file. The local scenario settings are read over the map's own, and its trigger types and tag types are registered with the map. Those triggers are what take the lighting from one hour to the next; the flag by itself schedules nothing.

```ini title="map seed file"
[RandomMap]
Time=1
UseTransitions=yes
```

The flag also changes the floodlights at the start points. Instead of the count the hour would give (none in the morning or afternoon, two at dusk, four at night), every start point is ringed with four. Each light is attached to the tag named `Light On/Off`, so the same triggers can switch them off by day and on again at night.

The generator asks no questions about the Firestorm addon here, so a seed file may set the flag whether or not that addon is present. The map generator dialog offers the flag as a check box and clears it outright when Firestorm is absent. Its randomize button sets it on about half the seeds it produces, and only under that addon.
