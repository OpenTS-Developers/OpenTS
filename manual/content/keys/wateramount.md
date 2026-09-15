---
key: WaterAmount
summary: How much of a generated map is put under water, as a figure from 0 to 100.
see_also: [Biome, Accessibility]
when_omitted:
  kind: value
  value: "0"
  note: The fallback of `0` skips the river and lake passes alike and leaves the map dry.
---

The figure is a budget rather than a share. It is multiplied by the playable area and by a factor chosen per biome, and a hundred cells are added on top. The river and lake passes each place a single feature against that total and then stop: the lake is sized to fit whatever remains after the river has taken its share. The per-biome factor is what makes the same figure wetter on tundra than on temperate and far drier on desert. Desert is given a third of what the next driest biome gets and a quarter of what tundra gets. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Biome=2
WaterAmount=45
```

At `0` the whole water pass is skipped and the map is left dry. Above `0` one lake is attempted, up to ten times before the generator gives up and settles for a drier map than was asked for. A river is attempted first, on the same ten-try basis, but only above `20` and never on the desert biome. The figure also widens the river's brush and lengthens an arctic river as it rises.
