---
key: VeinholeMonsters
summary: Veinhole monsters the random map generator plants.
see_also: ["system:veins", "VeinholeTypeClass"]
when_omitted:
  kind: value
  value: "0"
  note: The fallback of `0` asks for no veinhole monsters, so the placement pass plants none.
---

```ini title="map seed file"
[RandomMap]
VeinholeMonsters=3
```

The generator tries to plant this many veinhole monsters. When a map is [generated from a file](/systems/map-generation/#the-dialog-path-and-the-scenario-path), a value below `0` becomes `0` and one above `5` becomes `5`. The generator makes at most 200 placement attempts in total, so a crowded map can get fewer monsters than requested. Each attempt picks a five-by-five block. The block must be clear, level ground inside the playable area, with no overlay and no cell in the ground reserved around a start point, and its center must pass the [placement test](/systems/veins/#placement). Each monster planted starts with a ring of veins along the border of that block.

The pass runs only where the generated map is built in the temperate theater, so a map whose [`Biome`](/keys/biome/) is tundra or taiga is left without monsters whatever the figure says.
