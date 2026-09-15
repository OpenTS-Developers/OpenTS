---
format_id: map-seed
title: Map seed files
summary: Stores random-map generator inputs in a `[RandomMap]` section.
kind: file
filenames:
  - "*.SED"
key_scopes:
  - file: map seed file
    section:
      kind: literal
      name: RandomMap
source_files:
  - code/mapgen.cpp
  - code/scenario.cpp
---

The random-map dialog reads and writes one `[RandomMap]` section. Scenario loading recognizes the `.SED` extension, loads the section, and generates a map from its values. That file is the generator's own `RandMap.Sed`, written by the random-map dialog for the match and read back through the ordinary file layer. A `.SED` the player saves is a settings file rather than a scenario. It keeps to the [saved-games folder](/formats/save-games/), and the random-map dialog is what lists and loads it.

```ini title="MyMap.SED"
[RandomMap]
Description=Four-player temperate map
Width=1
Height=1
NumPlayers=4
Seed=12345
```

[`Width`](/keys/width/) and [`Height`](/keys/height/) are size indices from `0` through `3` rather than cell counts. Each is read as a fraction of the way between a smallest and a largest size: `0`, one third, two thirds, then `1`. [`NumPlayers`](/keys/numplayers/) decides which pair of figures those two ends are. The table gives both ends for every player count the generator has figures for; width and height are drawn from identical tables, so the same index written for both yields the same number of cells each way. Neither index means a size on its own; the table settles what each one comes to. `Width=3` is 100 cells for two players and 175 for eight.

| `NumPlayers` | Cells at index `0` | Cells at index `3` |
| --- | --- | --- |
| 2 | 50 | 100 |
| 3 | 65 | 115 |
| 4 | 75 | 128 |
| 5 | 85 | 140 |
| 6 | 100 | 160 |
| 7 | 120 | 170 |
| 8 | 135 | 175 |

An index of `1` or `2` lands one third or two thirds of the way between the two ends and is truncated to a whole number. The figures that come out are written as the playable area, the region `[Map] LocalSize=` declares; the playfield the generator writes around it, `[Map] Size=`, is four cells wider and twelve taller.

:::danger[A player count outside two to eight reads outside the tables]
The player count selects a row by its own value less two, and nothing on the scenario loading path bounds it. The clamping that holds the random-map dialog inside the tables is not run on a `.SED` opened as a scenario. A file with `NumPlayers=1` or `NumPlayers=9` reads its two ends from memory outside the tables. The map is then built to whatever dimensions came back.
:::
