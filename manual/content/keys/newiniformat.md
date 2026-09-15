---
key: NewINIFormat
summary: The layout revision the rest of the map file is read in.
when_omitted:
  kind: value
  value: "0"
  note: The read substitutes a literal 0 rather than keeping the value the previous map established, so an absent key always selects the oldest layout.
---

```ini title="map file"
[Basic]
NewINIFormat=4
```

The number is read from `[Basic]` before any of the map's contents and then read by each section that holds positions. Two thresholds matter.

Above `1`, the overlay layer is taken from the compressed `[OverlayPack]` and `[OverlayDataPack]` blocks. At `1` or below neither block is opened and there is no other path into the overlay layer. The map loads with no Tiberium, no walls and no overlay bridges.

At `4` or above, an object placement record stores its cell as two separate columns. The cell numbers that key the terrain and cell-tag sections are read as `x + y * 1000`. Below `4` a placement record stores a single packed number and a cell key is read as `x + y * 128`, a form no map wider than 128 cells can express.

The same threshold decides the cell a [Move to Cell](/mapping/missions/tmission-movecell/) script line names. It applies to every script the scenario brings in, both the ones the map declares and the ones it inherits from the rules' AI file. The line's number is decoded once, while the script is being read, because the format value does not travel into a save game and would not be there to read later. From then on the team acts on the decoded cell.

:::danger[Leaving the key out changes how every position in the map is read]
The value is not inherited from the previously loaded map and does not fall back on the modern layout: the absent key stores `0`. A map without the key loses its entire overlay layer. Every unit, infantryman, structure, terrain object and cell tag is placed at a position decoded for a 128-cell map, which puts it somewhere else or off the map entirely.
:::
