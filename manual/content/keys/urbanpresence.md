---
key: UrbanPresence
summary: How built up a generated map is, as a figure from 0 to 100.
see_also: [Biome, Vegetation]
when_omitted:
  kind: value
  value: "0"
  note: The fallback of `0` places no rural settlements and no urban areas.
---

What the figure buys depends on the biome. Tundra and taiga get rural settlements: a road junction on open ground with room around it, a handful of civilian buildings along the roads and a few civilian vehicles. Temperate, desert and mutated get full urban areas instead, each a paved district grown out from a cell with its own road network, buildings and traffic. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Biome=2
UrbanPresence=3
```

At `0` nothing is placed at all. Each further point asks for two more rural settlements or three more urban areas. The pass gives up after ten attempts however many were asked for. An attempt that finds nowhere suitable is spent all the same.

:::caution[Only the bottom of the range does anything]
Because of the ten-attempt ceiling, the figure stops making a difference once the number asked for reaches ten: on tundra and taiga that happens at `5`, and on the other three biomes at `4`. Every figure from there to `100` builds the same map. The map generator dialog offers the whole range as a slider regardless.
:::
