---
key: TemperateOccupationBits
summary: Which of a cell's three infantry standing places a terrain object fills in the temperate theater.
see_also: [SnowOccupationBits, Foundation]
when_omitted:
  kind: value
  value: "7"
  note: All three standing places are filled, which is also the figure that makes the cell fully blocked.
---

A cell has five sub-positions, and only three of them can hold a man: the north-east, the south-west and the south-east. This figure is a set of three bits, one per standing place, and the object fills each place whose bit is set: `1` for north-east, `2` for south-west, `4` for south-east. `6` therefore leaves the north-east corner open and blocks the other two.

```ini title="rules.ini"
[MYROCK]                     ; example boulder that infantry can squeeze past
TemperateOccupationBits=4    ; only the south-east standing place is filled
SnowOccupationBits=4
```

The figure has a second effect that is not read bit by bit. A cell holding a terrain object is classed as fully blocked when the figure is exactly `7`, and as partly blocked for any other figure (including `0`). The two classes are crossable by different [movement zones](/reference/enums/movement-zone/), so moving off `7` changes which of them can cross the cell: `Infantry` gains it, while `Destroyer`, `AmphibiousDestroyer` and `Subterannean` lose it. `InfantryDestroyer` and `Fly` cross either class, and `Normal`, `Crusher`, `Amphibious` and `AmphibiousCrusher` cross neither.

The figure applies in the temperate theater alone. [`SnowOccupationBits`](/keys/snowoccupationbits/) is read in the snow theater, and a type that sets one without the other changes behavior in one theater only.

:::caution[Only the object's own cell gets standing places filled]
The standing places are marked on the cell the terrain object is anchored to, and only there, however many cells its [`Foundation`](/keys/foundation/#scope-terraintype) covers. The remaining cells of a larger object never receive a mark, whatever the figure says, so on any figure other than `7` they are ground infantry may path into with all three places standing open.
:::
